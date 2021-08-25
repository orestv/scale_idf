#pragma once

namespace scale::state {
    enum State {
        SCALE_STATE_NORMAL,
        SCALE_STATE_TARE,
    };

    class StateMachine {
    public:
        StateMachine(): _state(SCALE_STATE_NORMAL) {}
        void setState(State state) {
            _state = state;
        }
        State state() const {
            return _state;
        }
    private:
        State _state;
    };
}