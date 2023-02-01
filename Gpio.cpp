/*
 *    Copyright (c) 2023 Sangchul Go <luke.go@hardkernel.com>
 *
 *    OdroidThings is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    OdroidThings is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with OdroidThings.
 *    If not, see <http://www.gnu.org/licenses/>.
 */

#include "Gpio.h"

Gpio::Gpio(std::shared_ptr<Board>  board): board(board) {
}

std::vector<std::string> Gpio::getList() {
    std::vector<std::string> list;
    auto pinList = board->getPinList();

    for (auto pin = pinList.begin(); pin != pinList.end(); pin++) {
        if (pin->availableModes & PIN_GPIO) {
            int alt = getAlt(pin->pin);
            if (alt < 2)
                list.push_back(pin->name);
        }
    }

    return list;
}

bool Gpio::getValue(int idx) {
    return (digitalRead(board->getPin(idx)) == HIGH);
}

void Gpio::setDirection(int idx, direction_t direction) {
    int pin = board->getPin(idx);
    switch (direction) {
        case DIRECTION_IN:
            pinMode(pin, INPUT);
            break;
        case DIRECTION_OUT_INITIALLY_HIGH:
            pinMode(pin, OUTPUT);
            digitalWrite(pin, HIGH);
            break;
        case DIRECTION_OUT_INITIALLY_LOW:
            pinMode(pin, OUTPUT);
            digitalWrite(pin, LOW);
            break;
    }
}

void Gpio::setValue(int idx, bool value) {
    digitalWrite(board->getPin(idx), value?HIGH:LOW);
}

void Gpio::setActiveType(int idx, int activeType) {
    int pin = board->getPin(idx);
    switch (activeType) {
        case ACTIVE_LOW:
            pullUpDnControl(pin, PUD_UP);
            break;
        case ACTIVE_HIGH:
            pullUpDnControl(pin, PUD_DOWN);
            break;
    }

}

void Gpio::setEdgeTriggerType(int idx, int edgeTriggerType) {
    int pin = board->getPin(idx);
    switch (edgeTriggerType) {
        case EDGE_NONE:
            return;
            break;
        case EDGE_RISING:
            triggerType[pin] = INT_EDGE_RISING;
            break;
        case EDGE_FALLING:
            triggerType[pin] = INT_EDGE_FALLING;
            break;
        case EDGE_BOTH:
            triggerType[pin] = INT_EDGE_BOTH;
            break;
    }
}

void Gpio::registerCallback(int idx, function_t callback) {
    int pin = board->getPin(idx);
    wiringPiISR(pin, triggerType[pin], callback);
}

void Gpio::unregisterCallback(int idx) {
    wiringPiISRCancel(board->getPin(idx));
}