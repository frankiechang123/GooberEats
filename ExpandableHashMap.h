// ExpandableHashMap.h
#include<list>
#include<string>


template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5);
	~ExpandableHashMap();
	void reset();
	int size() const;
	void associate(const KeyType& key, const ValueType& value);

	// for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	// for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	// C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
	struct Node {
		KeyType key;
		ValueType value;
	};
	double m_maxLoadFactor;
	std::list <Node*>* m_buckets;   //an array of list of Node pointers
	int m_nUsed;
	int m_nBuckets;

	void clearMap() {
		for (int i = 0; i < m_nBuckets; i++) {
			for (auto it = m_buckets[i].begin(); it != m_buckets[i].end(); it++) {
				delete* it;     //erase()?
			}
		}
		delete[] m_buckets;
	}
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType,ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
	if(maximumLoadFactor>0)
		m_maxLoadFactor = maximumLoadFactor;
	else 
		m_maxLoadFactor = 0.5;
	m_nBuckets = 8;
	m_nUsed = 0;
	m_buckets = new std::list<Node*>[m_nBuckets];
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType,ValueType>::~ExpandableHashMap()
{
	clearMap();
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::reset()
{
	clearMap();

	m_nBuckets = 8;
	m_nUsed = 0;
	m_buckets = new std::list<Node*>[m_nBuckets];  
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType,ValueType>::size() const
{
	int count = 0;
	for (int i = 0; i < m_nBuckets; i++) {
		count += m_buckets[i].size();
	}
	return count;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::associate(const KeyType& key, const ValueType& value)
{
	ValueType* v=find(key);
	if (v != nullptr) {
		*v = value;
		return;
	}
	unsigned int hasher(const KeyType & k);
	unsigned int hashKey = hasher(key);
	int pos = hashKey % m_nBuckets;
	Node* newNode = new Node();
	newNode->key = key;
	newNode->value = value;
	if (m_buckets[pos].empty()) 
		m_nUsed++;
	m_buckets[pos].push_back(newNode);

	if (m_nBuckets * m_maxLoadFactor < m_nUsed) {
		int newNBuckets = m_nBuckets * 2;
		m_nUsed = 0;

		std::list<Node*>* newBuckets = new std::list<Node*>[newNBuckets];
		for (int i = 0; i < m_nBuckets; i++) {
			for (auto it = m_buckets[i].begin(); it != m_buckets[i].end(); it++) {
				unsigned int hashKey = hasher((*it)->key);
				int pos = hashKey % (newNBuckets);
				/*Node* newNode = new Node();
				newNode->key = (*it)->key;
				newNode->value = (*it)->value;*/ //BUG--> could cause pointers returned by find invalidated
				if (newBuckets[pos].empty()) m_nUsed++;
				newBuckets[pos].push_back(*it);
			}
		}

		//clearmap(); //should not use clear map
		delete[] m_buckets; //free former array memory
		m_buckets = newBuckets; //point the pointer to the new array
		m_nBuckets = newNBuckets;
	}
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType,ValueType>::find(const KeyType& key) const
{
	//for (int i = 0; i < m_nBuckets; i++) {
	//	for (auto it = m_buckets[i].begin(); it != m_buckets[i].end(); it++) {
	//		if (key == (*it)->key) {
	//			return &((*it)->value);
	//		}
	//	}
	//}
	unsigned int hasher(const KeyType & k);
	unsigned int hashValue = hasher(key);
	int pos = hashValue % m_nBuckets;
	for (auto it = m_buckets[pos].begin(); it != m_buckets[pos].end(); it++) {
		if (key == (*it)->key)
			return &((*it)->value);
	}
	return nullptr;
}


