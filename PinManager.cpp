/*
 *    Copyright (c) 2020 Sangchul Go <luke.go@hardkernel.com>
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

#include "OdroidC4.h"
#include "OdroidM1.h"
#include "OdroidN2.h"
#include "PinManager.h"
#include <cutils/log.h>
#include <cutils/properties.h>
#include <hardware/odroidThings.h>
#include <hardware/odroidthings-base.h>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <wiringPi/wiringPi.h>

PinManager::PinManager(){
    char boardName[PROPERTY_VALUE_MAX];
    std::string name;
    isUnknown= false;

    property_get(BOARD_PROPERTY, boardName, NULL);
    name = boardName;

    if (!name.compare(0, strlen("odroidn2"), "odroidn2"))
        board = std::make_shared<OdroidN2>();
    else if (!name.compare(0, strlen("odroidc4"), "odroidc4"))
        board = std::make_shared<OdroidC4>();
    else if (!name.compare(0, strlen("odroidm1"), "odroidm1"))
        board = std::make_shared<OdroidM1>();
    else {
        board = std::make_shared<Board>(name);
        isUnknown= true;
        ALOGD("Board is not initialized");
        return;
    }

    ALOGD("Board is %s", name.c_str());
}

int PinManager::init() {
    if (isUnknownBoard())
        return -1;

    if (wiringPiSetup()) {
        ALOGD("Board is not initialized");
        return -1;
    }

    if (initPwm() < 0) {
        ALOGD("Board is not initialized");
        return  -1;
    }

    return 0;
}

bool PinManager::isUnknownBoard() {
    if (isUnknown)
        return true;

    std::ifstream modelName ("/proc/device-tree/model");
    if (modelName.fail())
        return true;

    std::string name;
    while(!modelName.eof()) {
        modelName >> name;
        if (name.find("ODROID") != std::string::npos)
            break;
    }
    modelName.close();

    std::size_t pos = name.find("-");
    name = name.substr(pos + 1);
    name.pop_back();

    for(const auto& model: models)
        if (model.find(name) != std::string::npos)
            return false;

    return true;
}

std::vector<pin_t> PinManager::getPinList() {
    return board->getPinList();
}

std::vector<std::string> PinManager::getPinNameList() {
    return board->getPinNameList();
}

std::vector<std::string> PinManager::getListOf(int mode) {
    std::vector<std::string> list;
    switch (mode) {
        case PIN_PWM: {
            auto pinList = board->getPinList();

            for (auto pin = pinList.begin(); pin != pinList.end(); pin++) {
                if (pin->availableModes & PIN_PWM) {
                    if (pwm.count(pin->pin) > 0) {
                        list.push_back(pin->name);
                    }
                }
            }
            break;
        }
    }
    return list;
}

#define PWM_RANGE_MAX 65535

int PinManager::initPwm() {
    auto list = board->getPwmList();
    for (auto pin = list.begin(); pin != list.end(); pin++)
        initPwmState(pin->index, pin->chip, pin->line);

    return 0;
}

void PinManager::initPwmState(int idx, uint8_t chip, uint8_t node) {
    const auto state = std::make_shared<pwmState>();
    // init chip & node info
    state->chip = chip;
    state->node = node;

    // init configuration sysfs path.
    std::ostringstream pwmRoot;

    pwmRoot << "/sys/class/pwm/pwmchip" << std::to_string(state->chip);
    if (access(pwmRoot.str().c_str(), F_OK) == 0) {
        pwmRoot << "/pwm" << std::to_string(state->node) <<"/";
        std::string pwmRootStr = pwmRoot.str();

        state->periodPath =  pwmRootStr + "period";
        state->dutyCyclePath = pwmRootStr + "duty_cycle";
        state->enablePath = pwmRootStr + "enable";

        pwm.insert(std::make_pair(idx, state));
    }
}

inline void PinManager::writeSysfsTo(const std::string path, const std::string value) {
    std::ofstream file(path);
    file << value;
    file.close();
}

void PinManager::openPwm(int idx) {
    auto pin = board->getPin(idx);
    const auto state = pwm.find(pin)->second;
    std::ostringstream openPath;

    ALOGD("openPwm(chip:%d-node:%d)", state->chip, state->node);

    openPath << "/sys/class/pwm/pwmchip" << std::to_string(state->chip) << "/";

    writeSysfsTo(openPath.str() + "export", std::to_string(state->node));

    state->unexportPath = openPath.str() + "unexport";
    openPath << "pwm" << std::to_string(state->node) << "/";
    writeSysfsTo(openPath.str() + "polarity", "normal");
}

void PinManager::closePwm(int idx) {
    auto pin = board->getPin(idx);
    const auto state = pwm.find(pin)->second;
    state->period = 0;
    state->cycle_rate = 0;

    ALOGD("closePwm(chip:%d-node:%d)", state->chip, state->node);

    writeSysfsTo(state->enablePath, "0");
    writeSysfsTo(state->dutyCyclePath, "0");
    writeSysfsTo(state->periodPath, "0");
    writeSysfsTo(state->unexportPath, std::to_string(state->node));
}

bool PinManager::setPwmEnable(int idx, bool enabled) {
    auto pin = board->getPin(idx);
    const auto state = pwm.find(pin)->second;

    writeSysfsTo(state->enablePath, (enabled?"1":"0"));
    return true;
}

bool PinManager::setPwmDutyCycle(int idx, double cycle_rate) {
    auto pin = board->getPin(idx);
    const auto state = pwm.find(pin)->second;
    unsigned int duty_cycle = (state->period / 100) * cycle_rate;

    writeSysfsTo(state->dutyCyclePath, std::to_string(duty_cycle));

    state->cycle_rate = cycle_rate;

    return true;
}

#define HERTZTONANOSECOND 1000000000
bool PinManager::setPwmFrequency(int idx, double frequency_hz) {
    auto pin = board->getPin(idx);
    const auto state = pwm.find(pin)->second;

    state->period = HERTZTONANOSECOND / frequency_hz;

    writeSysfsTo(state->periodPath, std::to_string(state->period));

    if (state->cycle_rate != 0) {
        setPwmDutyCycle(idx, state->cycle_rate);
    }

    return true;
}

std::unique_ptr<Gpio> PinManager::getGpio() {
    auto gpio = std::make_unique<Gpio>(board);
    return gpio;
}

std::unique_ptr<I2c> PinManager::getI2c() {
    auto i2cList = board->getI2cList();
    std::vector<i2c_t> list;
    for (auto i2c = i2cList.begin(); i2c != i2cList.end(); i2c++) {
        if (access(i2c->path.c_str(), F_OK) == 0)
            list.push_back(*i2c);
    }

    if (list.size()) {
        auto i2c = std::make_unique<I2c>(list);
        return i2c;
    } else
        return NULL;
}

std::unique_ptr<Uart> PinManager::getUart() {
    auto uartList = board->getUartList();
    std::vector<uart_t> list;
    for (auto uart = uartList.begin(); uart != uartList.end(); uart++) {
        if (access(uart->path.c_str(), F_OK) == 0)
            list.push_back(*uart);
    }

    if (list.size()) {
        auto uart = std::make_unique<Uart>(list);
        return uart;
    } else
        return NULL;
}

std::unique_ptr<Spi> PinManager::getSpi() {
    auto spiList = board->getSpiList();
    if (access(spiList[0].path.c_str(), F_OK) == 0) {
        std::unique_ptr<Spi> spi = std::make_unique<Spi>(spiList);
        return spi;
    }
    return NULL;
}
