#pragma once

#include <vector>
using std::vector;

namespace rain {
    namespace mem {
        template<typename T>
        class Pool {
            vector<T*> objects;
        public:
            Pool(int initial_capacity = 0)
                : objects(initial_capacity)
            {
            }

            void mark(T *token) {
                objects.push_back(token);
            }

            void cleanup() {
                for (T *obj : objects)
                    delete obj;
                objects = vector<T*>();
            }

            ~Pool() {
                cleanup();
            }
        };
    }
}