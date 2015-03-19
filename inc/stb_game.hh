#ifndef STB_GAME_HH_
#define STB_GAME_HH_

#include "stb_types.hh"

namespace stb
{

template<typename Impl, typename Duration, typename TimePoint>
class Main
{
public:
    Main(Impl & impl, const Duration logicStep, TimePoint(*f)())
        : m_impl(impl), m_logicStep(logicStep), getTime(f)
        {}

    U run() {
        TimePoint previousTime = getTime();
        Duration lag;

        for (;;) {
            U ret = 0;

            TimePoint currentTime = getTime();
            Duration elapsed = currentTime - previousTime;
            previousTime = currentTime;
            lag += elapsed;

            if ((ret = m_impl.handleInput()) != 0) {
                return ret;
            }

            while (lag >= m_logicStep) {
                m_impl.update(elapsed);
                lag -= m_logicStep;
            }

            m_impl.render();
        }
    }

private:
    Impl & m_impl;
    const Duration m_logicStep;
    TimePoint (*getTime)();

    Main(const Main &){}
    Main operator = (const Main &){ return *this; }
};

}

#endif
