#pragma once

#include "hash_set.h"

namespace Wide {
    namespace Utility {
        namespace details {
            template<typename T> class hash_first {
                hash_first(T pred)
                    : t(std::move(pred)) {}
                hash_first() {}
                hash_first(const hash_first& other)
                    : t(other.t) {}
                hash_first(hash_first&& other)
                    : t(std::move(other.t)) {}
                void swap(hash_first& other) {
                    using std::swap;
                    swap(t, other.t);
                }
                T t;
                template<typename H> std::size_t operator()(H&& h) const {
                    return t(std::forward<H>(h).first);
                }
            };
            template<typename T> class pred_first {        
                pred_first(T pred)
                    : t(std::move(pred)) {}
                pred_first() {}
                pred_first(const pred_first& other)
                    : t(other.t) {}
                pred_first(pred_first&& other)
                    : t(std::move(other.t)) {}
                void swap(pred_first& other) {
                    using std::swap;
                    swap(t, other.t);
                }
        
                T t;
                template<typename A1, typename A2> std::size_t operator()(A1&& a1, A2&& a2) const {
                    return t(std::forward<A1>(a1).first, std::forward<A2>(a2).first);
                }
            };
        }
        
        template<typename K, typename V, typename H = std::hash<K>, typename P = std::equal_to<K>, typename A = std::allocator<std::pair<const K, V>>> class hash_map 
        : private hash_set<std::pair<const K, V>, details::hash_first<H>, details::pred_first<P>, A> {
        public:
            hash_map() {}
            hash_map(const hash_map& other) : hash_set(other) {}
            hash_map(hash_map&& other) : hash_set(std::move(other)) {}
        
            hash_map& operator=(const hash_map& other) {
                hash_set::operator=(other);
                return *this;
            }
            hash_map& operator=(hash_map&& other) {
                hash_set::operator=(std::move(other));
                return *this;
            }
        
            using hash_set::begin;
            using hash_set::clear;
            using hash_set::empty;
            using hash_set::end;
            using hash_set::erase;
            using hash_set::find;
            using hash_set::insert;
            using hash_set::load_factor;
            using hash_set::max_load_factor;
            using hash_set::rehash;
            using hash_set::size;
            using hash_set::swap;    
        
            template<typename AK> V& operator[](AK&& ak) {
                auto it = find(ak);
                if (it == end())
                    return insert(std::make_pair(std::forward<AK>(ak), V())).first->second;
                return it->second;
            }
        };
    }
}