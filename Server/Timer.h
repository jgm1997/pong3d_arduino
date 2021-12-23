#include <Arduino.h>

#ifndef TIMER_H
#define TIMER_H

class Timer {
    public:
        Timer(uint8_t timer);

        // Setters
        void setTime(uint16_t time_ms);
        void setFunction(void (*function)());

        void load();
        void action();

    private:
        // Attributes
        uint8_t timer;
        uint8_t clock_select;
        uint16_t output_compare;
        void (*function)();

        // Private methods
        uint8_t getBits(uint8_t timer);
};

#endif /* TIMER_H */