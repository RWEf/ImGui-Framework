#pragma once
#include <thread>

namespace Hook {

    using HookType = void(*)();

    class HookInit {

    private:

        using HookType = void(*)();

    private:

        void* Function1 = nullptr;
        void* Function2 = nullptr;
        void* Function3 = nullptr;
        void* Function4 = nullptr;

    public:

        void* operator[](size_t sub) {
            if (sub > 3)
                return nullptr;

            if (sub == 0) {
                return &Function1;
            }
            else {
                size_t offset = ((sub + 1) * sizeof(void*)) - sizeof(void*);

                return (void*)((size_t)(this) + offset);
            }
        }

        void operator()(size_t sub) {
            if (sub > 3)
                return;

            size_t offset;

            if (sub == 0) {
                return ((HookType)Function1)();
            }
            else {
                offset = ((sub + 1) * sizeof(void*)) - sizeof(void*);
            }

            void* CallAddress = (void*)((size_t)(this) + offset);
            HookType Call = (HookType)(*(size_t*)(CallAddress));
            return Call();
        }

    };

    struct HookRander {
        void* Function1 = nullptr;
        void* Function2 = nullptr;

        void ThreadCall(size_t Target) {

            switch (Target)
            {
            case 0:
                if (Function1 == nullptr)
                    return;
                
                std::thread((HookType)(Function1)).join();
                
                return;
            case 1:
                if (Function2 == nullptr)
                    return;
                
                std::thread((HookType)(Function2)).join();
                
                return;
            default:
                return;
            }
        }
    };

}