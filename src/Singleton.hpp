#pragma once

#define SINGLETON(class_name) \
        public: \
        static class_name& getInstance() { \
            static class_name instance; \
            return instance; \
        } \
        private: \
        class_name() = default;

