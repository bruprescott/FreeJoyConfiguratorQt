#include "pinconfig.h"
#include "ui_pinconfig.h"
#include <QLabel>
#include <QDebug>

#include "global.h"
#include <QSettings>

PinConfig::PinConfig(QWidget *parent) :         // пины - первое, что я начал кодить в конфигураторе и спустя время
    QWidget(parent),                            // заявляю - это говнокод!1 который даже мне тяжело понять
    ui(new Ui::PinConfig),                      // мои соболезнования тем кто будет разбираться)
    m_bluePill{new PinsBluePill(this)},
    m_contrLite{new PinsContrLite(this)}
{
    ui->setupUi(this);
    m_bluePill->hide();
    m_contrLite->hide();

    m_maxButtonsWarning = false;
    m_shiftLatchCount = 0;
    m_shiftDataCount = 0;

    // create pin combo box. i+1! start from 1
    // возможно использовать одни и те же комбобоксы пинов в разных виджетах плат - изврат,
    // но каждый виджет плат со своими комбобоксами тоже не лучший вариант.
    // 99% польователей будут использовать Blue Pill и им постоянно придётся
    // таскать комбобоксы с Controller Lite, которые, как минимум, увеличат время запуска приложения
    for (int i = 0; i < PINS_COUNT; ++i) {
        PinComboBox *pinComboBox = new PinComboBox(i+1, this);
        pinComboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        m_pinCBoxPtrList.append(pinComboBox);
    }

    gEnv.pAppSettings->beginGroup("BoardSettings");
    m_lastBoard = gEnv.pAppSettings->value("SelectedBoard", 0).toInt();
    if (m_lastBoard < 0 || m_lastBoard > 1) {
        m_lastBoard = 0;
    }
    gEnv.pAppSettings->endGroup();

    if (m_lastBoard == 0) {
        m_bluePill->addPinComboBox(m_pinCBoxPtrList);
        ui->layoutV_pins->addWidget(m_bluePill);
        m_bluePill->show();
    } else if (m_lastBoard == 1) {
        m_contrLite->addPinComboBox(m_pinCBoxPtrList);
        ui->layoutV_pins->addWidget(m_contrLite);
        m_contrLite->show();
    }
    ui->comboBox_board->addItem("Blue Pill");
    ui->comboBox_board->addItem("Controller Lite");
    ui->comboBox_board->setCurrentIndex(m_lastBoard);
    connect(ui->comboBox_board, SIGNAL(currentIndexChanged(int)),
            this, SLOT(boardChanged(int)));

    for (int i = 0; i < m_pinCBoxPtrList.size(); ++i) {
            connect(m_pinCBoxPtrList[i], SIGNAL(valueChangedForInteraction(int, int, int)),       // valgrind сообщает о утечке, но почему?
                        this, SLOT(pinInteraction(int, int, int)));
            connect(m_pinCBoxPtrList[i], SIGNAL(currentIndexChanged(int, int, int)),
                        this, SLOT(pinIndexChanged(int, int, int)));
    }

    connect(ui->widget_currConfig, SIGNAL(totalButtonsValueChanged(int)),
            this, SIGNAL(totalButtonsValueChanged(int)));
    connect(ui->widget_currConfig, SIGNAL(totalLEDsValueChanged(int)),
            this, SIGNAL(totalLEDsValueChanged(int)));

}

PinConfig::~PinConfig()
{
    gEnv.pAppSettings->beginGroup("BoardSettings");
    gEnv.pAppSettings->setValue("SelectedBoard", m_lastBoard);
    gEnv.pAppSettings->endGroup();
    delete ui;
}

void PinConfig::retranslateUi()
{
    ui->retranslateUi(this);
    for (int i = 0; i < m_pinCBoxPtrList.size(); ++i) {
        m_pinCBoxPtrList[i]->retranslateUi();
    }
}


void PinConfig::boardChanged(int index)
{
    if (index == 0 && m_lastBoard == 1) {
        Q_ASSERT(qobject_cast<PinsContrLite *>(ui->layoutV_pins->takeAt(0)->widget()));
        m_contrLite->hide();
        ui->layoutV_pins->takeAt(0);

        m_bluePill->addPinComboBox(m_pinCBoxPtrList);
        ui->layoutV_pins->addWidget(m_bluePill);
        m_bluePill->show();
        m_lastBoard = 0;
    } else if (index == 1 && m_lastBoard == 0) {
        Q_ASSERT(qobject_cast<PinsBluePill *>(ui->layoutV_pins->takeAt(0)->widget()));
        m_bluePill->hide();
        ui->layoutV_pins->takeAt(0);

        m_contrLite->addPinComboBox(m_pinCBoxPtrList);
        ui->layoutV_pins->addWidget(m_contrLite);
        m_contrLite->show();
        m_lastBoard = 1;
    }
}

void PinConfig::pinInteraction(int index, int senderIndex, int pin)
{
    if (index != NOT_USED) {//current_enum_index
        for (int i = 0; i < m_pinCBoxPtrList.size(); ++i) {
            for (int j = 0; j < m_pinCBoxPtrList[i]->pinTypeIndex().size(); ++j) {
                if (m_pinCBoxPtrList[i]->pinTypeIndex()[j] == index)
                {
                    if(m_pinCBoxPtrList[i]->interactCount() == 0){
                        m_pinCBoxPtrList[i]->setInteractCount(m_pinCBoxPtrList[i]->interactCount() + pin);
                        m_pinCBoxPtrList[i]->setIndex_iteraction(j, senderIndex);
                    }
                    else if (m_pinCBoxPtrList[i]->isInteracts() == true){
                        m_pinCBoxPtrList[i]->setInteractCount(m_pinCBoxPtrList[i]->interactCount() + pin);
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < m_pinCBoxPtrList.size(); ++i) {
            if (m_pinCBoxPtrList[i]->isInteracts() == true)
            {
                for (int j = 0; j < m_pinCBoxPtrList[i]->pinTypeIndex().size(); ++j) {
                    if (m_pinCBoxPtrList[i]->pinTypeIndex()[j] == senderIndex)
                    {
                        if(m_pinCBoxPtrList[i]->interactCount() > 0){
                            m_pinCBoxPtrList[i]->setInteractCount(m_pinCBoxPtrList[i]->interactCount() - pin);
                        }
                        if (m_pinCBoxPtrList[i]->interactCount() <= 0) {
                            m_pinCBoxPtrList[i]->setIndex_iteraction(0, senderIndex);
                        }
                    }
                }
            }
        }
    }
}


void PinConfig::pinIndexChanged(int currentDeviceEnum, int previousDeviceEnum, int pinNumber)  // мб сделать сразу запись в конфиг из пинов
{                                                                                              // или отдельный класс для их состояний
    // signals for another widgets
    signalsForWidgets(currentDeviceEnum, previousDeviceEnum, pinNumber);

    // pin type limit  // переизбыток функционала(изи менять в структуре), не думаю, что понадобится в будущем, можно было и захардкодить
    pinTypeLimit(currentDeviceEnum, previousDeviceEnum);

    // set current config and generate signals for another widgets
//    else {
    setCurrentConfig(currentDeviceEnum, previousDeviceEnum, pinNumber);
}


void PinConfig::signalsForWidgets(int currentDeviceEnum, int previousDeviceEnum, int pinNumber)
{
    //fast encoder selected
    if (currentDeviceEnum == FAST_ENCODER){
        emit fastEncoderSelected(m_pinCBoxPtrList[0]->pinList()[pinNumber - PA_0].guiName, true);    // hz
    } else if (previousDeviceEnum == FAST_ENCODER){
        emit fastEncoderSelected(m_pinCBoxPtrList[0]->pinList()[pinNumber - PA_0].guiName, false);    // hz
    }
    // shift register latch selected
    if (currentDeviceEnum == SHIFT_REG_LATCH){
        m_shiftLatchCount++;
        emit shiftRegSelected(pinNumber, 0, m_pinCBoxPtrList[0]->pinList()[pinNumber - PA_0].guiName);    // hz
    } else if (previousDeviceEnum == SHIFT_REG_LATCH){
        m_shiftLatchCount--;
        emit shiftRegSelected((pinNumber)*-1, 0, m_pinCBoxPtrList[0]->pinList()[pinNumber - PA_0].guiName);    // hz
    }
    // shift register data selected
    if (currentDeviceEnum == SHIFT_REG_DATA){
        m_shiftDataCount++;
        emit shiftRegSelected(0, pinNumber, m_pinCBoxPtrList[0]->pinList()[pinNumber - PA_0].guiName);    // hz
    } else if (previousDeviceEnum == SHIFT_REG_DATA){
        m_shiftDataCount--;
        emit shiftRegSelected(0, (pinNumber)*-1, m_pinCBoxPtrList[0]->pinList()[pinNumber - PA_0].guiName);    // hz
    }
    // I2C selected
    if (currentDeviceEnum == I2C_SCL){// || current_device_enum == I2C_SDA){
        emit axesSourceChanged(-2, true);                                            // -2 enum I2C в axes.h
    } else if (previousDeviceEnum == I2C_SCL){// || previous_device_enum == I2C_SDA){
        emit axesSourceChanged(-2, false);
    }
}

void PinConfig::pinTypeLimit(int currentDeviceEnum, int previousDeviceEnum)
{
    static int limit_count_array[PIN_TYPE_LIMIT_COUNT]{};
    static bool limit_is_enable[PIN_TYPE_LIMIT_COUNT]{};

    for (int i = 0; i < PIN_TYPE_LIMIT_COUNT; ++i)
    {
        if (currentDeviceEnum == m_pinTypeLimit[i].deviceEnumIndex)
        {
            limit_count_array[i]++;
        }
        if (previousDeviceEnum == m_pinTypeLimit[i].deviceEnumIndex)
        {
            limit_count_array[i]--;
        }

        if (limit_count_array[i] >= m_pinTypeLimit[i].maxCount && limit_is_enable[i] == false)
        {
            limit_is_enable[i] = true;
            for (int j = 0; j < m_pinCBoxPtrList.size(); ++j)
            {
                for (int k = 0; k < m_pinCBoxPtrList[j]->enumIndex().size(); ++k) {
                    if (m_pinCBoxPtrList[j]->enumIndex()[k] == m_pinTypeLimit[i].deviceEnumIndex &&
                        m_pinCBoxPtrList[j]->currentDevEnum() != currentDeviceEnum)
                    {
                        m_pinCBoxPtrList[j]->setIndexStatus(int(k), false);
                    }
                }
            }
        }

        if (limit_is_enable[i] == true && limit_count_array[i] < m_pinTypeLimit[i].maxCount)
        {
            limit_is_enable[i] = false;
            for (int j = 0; j < m_pinCBoxPtrList.size(); ++j)
            {
                for (int k = 0; k < m_pinCBoxPtrList[j]->enumIndex().size(); ++k) {
                    if (m_pinCBoxPtrList[j]->enumIndex()[k] == m_pinTypeLimit[i].deviceEnumIndex)
                    {
                        m_pinCBoxPtrList[j]->setIndexStatus(int(k), true);
                    }
                }
            }
        }
    }
}

void PinConfig::setCurrentConfig(int currentDeviceEnum, int previousDeviceEnum, int pinNumber)
{
    for (int i = 0; i < SOURCE_COUNT; ++i) {
        for (int j = 0; j < PIN_TYPE_COUNT; ++j) {
            if(m_source[i].pinType[j] == 0){
                break;
            }
            else if(m_source[i].pinType[j] == currentDeviceEnum || m_source[i].pinType[j] == previousDeviceEnum){

                int tmp;
                if (m_source[i].pinType[j] == currentDeviceEnum){
                    tmp = 1;
                } else {
                    tmp = -1;
                }

                if (i == AXIS_SOURCE){      //int source_enum, bool is_add      axesSourceChanged
                    if (tmp > 0){
                        emit axesSourceChanged(pinNumber - 1, true);
                    } else {
                        emit axesSourceChanged(pinNumber - 1, false);
                    }
                }
                ui->widget_currConfig->setConfig(i, tmp);
            }
        }
    }
}

void PinConfig::shiftRegButtonsCountChanged(int count)
{
    ui->widget_currConfig->shiftRegButtonsCountChanged(count);
}

void PinConfig::a2bCountChanged(int count)
{
    ui->widget_currConfig->a2bCountChanged(count);
}



void PinConfig::resetAllPins()
{
    for (int i = 0; i < m_pinCBoxPtrList.size(); ++i) {
        m_pinCBoxPtrList[i]->resetPin();
    }
}


void PinConfig::readFromConfig(){
    for (int i = 0; i < m_pinCBoxPtrList.size(); ++i) {
        m_pinCBoxPtrList[i]->readFromConfig(i);
    }
}

void PinConfig::writeToConfig(){
    for (int i = 0; i < m_pinCBoxPtrList.size(); ++i) {
        m_pinCBoxPtrList[i]->writeToConfig(i);
    }
}

