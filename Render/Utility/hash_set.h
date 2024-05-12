#pragma once

#include <list>
#include <vector>

namespace Wide {
    namespace Utility {
        template<typename T, typename H = std::hash<T>, typename P = std::equal_to<T>, typename A = std::allocator<T>> class hash_set {
            struct item {
                T data;
                std::size_t cached_hash;
                item(T t, std::size_t hash)
                    : data(std::move(t)), cached_hash(hash) {}
            };
            std::list<item, A> data;
            H hash;
            P predicate;
            static const auto items_per_bucket = 5; // Arbitrary- determine by profile if performance undesirable
        public:    
            struct iterator {
                iterator(typename std::list<item, A>::iterator arg)
                    : it(arg) {}
        
                typename std::list<item, A>::iterator it;
        
                T& operator*() { return it->data; }
                T* operator->() { return &it->data; }
        
                bool operator==(iterator other) {
                    return it == other.it;
                }
                bool operator!=(iterator other) {
                    return it != other.it;
                }
        
                iterator operator++(int) {
                    auto ret(*this);
                    ++it;
                    return ret;
                }
                iterator& operator++() {
                    ++it;
                    return *this;
                }
                iterator operator--(int) {
                    auto ret(*this);
                    --it;
                    return ret;
                }
                iterator& operator--() {
                    --it;
                    return *this;
                }
        
            };
        
            iterator begin() { return iterator(data.begin()); }
            iterator end() { return iterator(data.end()); }
            std::size_t size() const { return data.size(); }
            void clear() { data.clear(); buckets.clear(); }
        
        private:
            std::vector<std::vector<iterator>> buckets;    
            void insert_into_buckets(iterator it) {
                if (buckets.size() == 0)
                    rehash(10); // Arbitrary- determine by profile if performance undesirable
                if (load_factor() > 0.75) {
                    rehash(buckets.size() * 2);
                    return insert_into_buckets(it);
                }
                if (buckets[it.it->cached_hash % buckets.size()].size() >= items_per_bucket) {
                    rehash(buckets.size() * 2);
                    return insert_into_buckets(it);
                }
                buckets[it.it->cached_hash % buckets.size()].push_back(it);
            }
        public:
            float load_factor() {
                return (float)size() / (float)buckets.size();
            }
            float max_load_factor() {
                return items_per_bucket;
            }
        
            void rehash(std::size_t size) {
                auto oldbuckets = std::move(buckets);
                buckets.resize(size);
                for(auto&& iterators : oldbuckets) {
                    for(auto&& i : iterators) {
                        insert_into_buckets(i);
                    }
                }
            }
        
            bool empty() { return data.size() == 0; }
        
            std::pair<iterator, bool> insert(T t) {
                auto it = find(t);
                if (it == end()) {
                    std::size_t val = hash(t);
                    data.push_back(item( std::move(t), val));
                    try {
                        insert_into_buckets(--data.end());
                    } catch(...) {
                        data.pop_back();
                        throw;
                    }
                    return std::make_pair(--data.end(), true);
                }
                return std::make_pair(it, false);
            }
            
            template<typename AK> iterator find(AK&& k) {
                return find(std::forward<AK>(k), hash, predicate);
            }
            template<typename AK, typename AH> iterator find(AK&& k, AH&& h) {
                return find(std::forward<AK>(k), std::forward<AH>(h), predicate);
            }
            template<typename AK, typename AH, typename AP> iterator find(AK&& k, AH&& h, AP&& p) {
                if (empty()) return end();
                auto&& bucket = buckets[h(k) % buckets.size()];
                for(auto it : bucket) {
                    if (p(k, it.it->data))
                        return it;
                }
                return end();
            }
        
            template<typename AK> iterator erase(AK&& k) {
                return erase(find(std::forward<AK>(k), hash, predicate));
            }
            template<typename AK, typename AH> iterator erase(AK&& k, AH&& h) {
                return erase(find(std::forward<AK>(k), std::forward<AH>(h), predicate));
            }
            template<typename AK, typename AH, typename AP> iterator erase(AK&& k, AH&& h, AP&& p) {
                return erase(find(std::forward<AK>(k), std::forward<AH>(h), std:forward<AP>(p)));
            }
        
            iterator erase(iterator it) {
                if (empty()) return end();
                auto&& bucket = buckets[it.it->cached_hash % buckets.size()];
                for(int i = 0; i < bucket.size(); i++) {
                    if (bucket[i] == it) {
                        bucket.erase(bucket.begin() + i);
                        return data.erase(it.it);
                    }
                }
                return end();
            }
        
            void swap(hash_set& other) {
                std::swap(data, other.data);
                std::swap(buckets, other.buckets);
                std::swap(hash, other.hash);
                std::swap(predicate, other.predicate);
            }
        
            hash_set& operator=(hash_set h) {
                swap(h);
                return *this;
            }
            hash_set(hash_set&& other)
                : data(std::move(other.data))
                , buckets(std::move(other.buckets))
            {}
            hash_set(const hash_set& other)
                : data(other.data) 
            {
                buckets.resize(other.buckets.size());
                for(auto it = data.begin(); it != data.end(); ++it)
                    insert_into_buckets(it);
            }
            hash_set() {}
        };
    }
}