#include "buttonconfig.h"
#include "ui_buttonconfig.h"
#include <QTimer>

#include <QDebug>
ButtonConfig::ButtonConfig(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ButtonConfig)
{
    ui->setupUi(this);
    m_isShifts_act = false;
    m_shift1_act = false;
    m_shift2_act = false;
    m_shift3_act = false;
    m_shift4_act = false;
    m_shift5_act = false;

    // make dynamic spawn?
    for (int i = 0; i < MAX_BUTTONS_NUM; i++) {
        ButtonLogical *logical_buttons_widget = new ButtonLogical(i, this);
        ui->layoutV_LogicalButton->addWidget(logical_buttons_widget);
        m_logicButtonPtrList.append(logical_buttons_widget);

        connect(m_logicButtonPtrList[i], SIGNAL(functionIndexChanged(int, int, int)),
                this, SLOT(functionTypeChanged(int, int, int)));
    }

    logicaButtonslCreator();
}

ButtonConfig::~ButtonConfig()
{
    delete ui;
}

void ButtonConfig::retranslateUi()
{
    ui->retranslateUi(this);
    for (int i = 0; i < m_logicButtonPtrList.size(); ++i) {
        m_logicButtonPtrList[i]->retranslateUi();
    }
}

// dynamic initialization of widgets. its decrease app startup time
// в идеале надо и создавать виджеты здесь, но возникает проблема - долго открывается вкладка
// и пока не знаю как это решить
void ButtonConfig::logicaButtonslCreator()
{
    static int tmp = 0;
    if (tmp >= MAX_BUTTONS_NUM) {
        if (MAX_BUTTONS_NUM != 128) {
            qCritical() << "buttonconfig.cpp MAX_BUTTONS_NUM != 128";
        }
        qDebug() << "LogicaButtonslCreator() finished";
        emit logicalButtonsCreated();
        return;
    }
    // как я понял таймер срабатывает после полной загрузки приложения(оно отобразится)
    // т.к. в LogicaButtonslCreator заходит при инициализации, но срабатывает после запуска приложения
    QTimer::singleShot(10, [&] {
        for (int i = 0; i < 8; i++) // MAX_BUTTONS_NUM(128)/8 = 16 ДОЛЖНО ДЕЛИТЬСЯ БЕЗ ОСТАТКА
        {
            m_logicButtonPtrList[tmp]->initialization();
            tmp++;
        }
        logicaButtonslCreator();
    });
}

void ButtonConfig::physButtonsSpawn(int count)
{
//    if (count > MAX_BUTTONS_NUM) {
//        count = MAX_BUTTONS_NUM;
//    }
//    if (count == m_PhysButtonPtrList.size()) {
//        return;
//    }
    // delete all
    while (!m_PhysButtonPtrList.empty()) {
        QWidget *widget = m_PhysButtonPtrList.takeLast();
        ui->layoutG_PhysicalButton->removeWidget(widget);
        widget->deleteLater();
    }
    // add
    int row = 0;
    int column = 0;
    ui->layoutG_PhysicalButton->setAlignment(Qt::AlignTop);
    for (int i = 0; i < count; i++) {
        if (column >= 8) // phys buttons column
        {
            row++;
            column = 0;
        }
        ButtonPhysical *physical_button_widget = new ButtonPhysical(i, this);
        ui->layoutG_PhysicalButton->addWidget(physical_button_widget, row, column);
        m_PhysButtonPtrList.append(physical_button_widget);
        column++;
    }
}

void ButtonConfig::functionTypeChanged(int index, int functionPreviousIndex, int buttonNumber)
{
    if (index == ENCODER_INPUT_A) {
        emit encoderInputChanged(buttonNumber + 1, 0);
    } else if (index == ENCODER_INPUT_B) {
        emit encoderInputChanged(0, buttonNumber + 1);
    }

    if (functionPreviousIndex == ENCODER_INPUT_A) {
        emit encoderInputChanged((buttonNumber + 1) * -1, 0); // send negative number
    } else if (functionPreviousIndex == ENCODER_INPUT_B) {
        emit encoderInputChanged(0, (buttonNumber + 1) * -1);
    }
}

void ButtonConfig::setUiOnOff(int value)
{
    if (value > 0) {
        ui->spinBox_Shift1->setEnabled(true);
        ui->spinBox_Shift2->setEnabled(true);
        ui->spinBox_Shift3->setEnabled(true);
        ui->spinBox_Shift4->setEnabled(true);
        ui->spinBox_Shift5->setEnabled(true);
    } else {
        ui->spinBox_Shift1->setEnabled(false);
        ui->spinBox_Shift2->setEnabled(false);
        ui->spinBox_Shift3->setEnabled(false);
        ui->spinBox_Shift4->setEnabled(false);
        ui->spinBox_Shift5->setEnabled(false);
    }
    for (int i = 0; i < m_logicButtonPtrList.size(); ++i) {
        m_logicButtonPtrList[i]->setMaxPhysButtons(value);
        m_logicButtonPtrList[i]->setSpinBoxOnOff(value);
    }

    physButtonsSpawn(value);
}

void ButtonConfig::buttonStateChanged()
{
    int number = 0;

    // logical buttons state
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            number = j + (i) *8;
            if ((gEnv.pDeviceConfig->gamepadReport.button_data[i] & (1 << (j & 0x07)))) {
                if (number < m_logicButtonPtrList.size()) {
                    m_logicButtonPtrList[number]->setButtonState(true);
                }
            } else if ((gEnv.pDeviceConfig->gamepadReport.button_data[i] & (1 << (j & 0x07))) == false) {
                if (number < m_logicButtonPtrList.size()) {
                    m_logicButtonPtrList[number]->setButtonState(false);
                }
            }
        }
    }

    // physical button state
    for (int i = 1; i < 9; i++) {
        for (int j = 0; j < 8; j++) {
            number = gEnv.pDeviceConfig->gamepadReport.raw_button_data[0] + j + (i - 1) * 8; //number = 64 + j + (i-1)*8;

            if ((gEnv.pDeviceConfig->gamepadReport.raw_button_data[i] & (1 << (j & 0x07)))) {
                if (number < m_PhysButtonPtrList.size()) {
                    m_PhysButtonPtrList[number]->setButtonState(true);
                }
            } else if ((gEnv.pDeviceConfig->gamepadReport.raw_button_data[i] & (1 << (j & 0x07))) == false) {
                if (number < m_PhysButtonPtrList.size()) {
                    m_PhysButtonPtrList[number]->setButtonState(false);
                }
            }
        }
    }

    // shift state
    for (int i = 0; i < SHIFT_COUNT; ++i) // выглядит как избыточный код, но так необходимо для оптимизации
    {
        if (gEnv.pDeviceConfig->gamepadReport.shift_button_data & (1 << (i & 0x07))) {
            m_isShifts_act = true;

            if (i == 0 && m_shift1_act == false) {
                m_defaultShiftStyle = ui->text_shift1_logicalButton->styleSheet();
                ui->text_shift1_logicalButton->setStyleSheet(m_defaultShiftStyle + "background-color: rgb(0, 128, 0);");
                //ui->groupBox_Shift1->setStyleSheet("background-color: rgb(0, 128, 0);");
                //ui->spinBox_Shift1->setStyleSheet("background-color: rgb(0, 128, 0);");
                m_shift1_act = true;
            } else if (i == 1 && m_shift2_act == false) {
                m_defaultShiftStyle = ui->text_shift1_logicalButton->styleSheet();
                ui->text_shift2_logicalButton->setStyleSheet(m_defaultShiftStyle + "background-color: rgb(0, 128, 0);");
                m_shift2_act = true;
            } else if (i == 2 && m_shift3_act == false) {
                m_defaultShiftStyle = ui->text_shift1_logicalButton->styleSheet();
                ui->text_shift3_logicalButton->setStyleSheet(m_defaultShiftStyle + "background-color: rgb(0, 128, 0);");
                m_shift3_act = true;
            } else if (i == 3 && m_shift4_act == false) {
                m_defaultShiftStyle = ui->text_shift1_logicalButton->styleSheet();
                ui->text_shift4_logicalButton->setStyleSheet(m_defaultShiftStyle + "background-color: rgb(0, 128, 0);");
                m_shift4_act = true;
            } else if (i == 4 && m_shift5_act == false) {
                m_defaultShiftStyle = ui->text_shift1_logicalButton->styleSheet();
                ui->text_shift5_logicalButton->setStyleSheet(m_defaultShiftStyle + "background-color: rgb(0, 128, 0);");
                m_shift5_act = true;
            }

        } else if (m_isShifts_act == true) {
            if (i == 0 && m_shift1_act == true) {
                ui->text_shift1_logicalButton->setStyleSheet(m_defaultShiftStyle);
                m_shift1_act = false;
            } else if (i == 0 && m_shift2_act == true) {
                ui->text_shift2_logicalButton->setStyleSheet(m_defaultShiftStyle);
                m_shift2_act = false;
            } else if (i == 0 && m_shift3_act == true) {
                ui->text_shift3_logicalButton->setStyleSheet(m_defaultShiftStyle);
                m_shift3_act = false;
            } else if (i == 0 && m_shift4_act == true) {
                ui->text_shift4_logicalButton->setStyleSheet(m_defaultShiftStyle);
                m_shift4_act = false;
            } else if (i == 0 && m_shift5_act == true) {
                ui->text_shift5_logicalButton->setStyleSheet(m_defaultShiftStyle);
                m_shift5_act = false;
            }

            if (m_shift1_act == false && m_shift2_act == false && m_shift3_act == false && m_shift4_act == false
                && m_shift5_act == false) {
                m_isShifts_act = false;
            }
        }
    }
}

void ButtonConfig::readFromConfig()
{
    dev_config_t *devc = &gEnv.pDeviceConfig->config;
    // logical buttons
    for (int i = 0; i < m_logicButtonPtrList.size(); i++) {
        m_logicButtonPtrList[i]->readFromConfig();
    }
    // other
    ui->spinBox_Shift1->setValue(devc->shift_config[0].button + 1);
    ui->spinBox_Shift2->setValue(devc->shift_config[1].button + 1);
    ui->spinBox_Shift3->setValue(devc->shift_config[2].button + 1);
    ui->spinBox_Shift4->setValue(devc->shift_config[3].button + 1);
    ui->spinBox_Shift5->setValue(devc->shift_config[4].button + 1);

    ui->spinBox_Timer1->setValue(devc->button_timer1_ms);
    ui->spinBox_Timer2->setValue(devc->button_timer2_ms);
    ui->spinBox_Timer3->setValue(devc->button_timer3_ms);

    ui->spinBox_DebounceTimer->setValue(devc->button_debounce_ms);
    ui->spinBox_A2bDebounce->setValue(devc->a2b_debounce_ms);

    ui->spinBox_EncoderPressTimer->setValue(devc->encoder_press_time_ms);
}

void ButtonConfig::writeToConfig()
{
    dev_config_t *devc = &gEnv.pDeviceConfig->config;
    devc->shift_config[0].button = ui->spinBox_Shift1->value() - 1;
    devc->shift_config[1].button = ui->spinBox_Shift2->value() - 1;
    devc->shift_config[2].button = ui->spinBox_Shift3->value() - 1;
    devc->shift_config[3].button = ui->spinBox_Shift4->value() - 1;
    devc->shift_config[4].button = ui->spinBox_Shift5->value() - 1;

    devc->button_timer1_ms = ui->spinBox_Timer1->value();
    devc->button_timer2_ms = ui->spinBox_Timer2->value();
    devc->button_timer3_ms = ui->spinBox_Timer3->value();

    devc->button_debounce_ms = ui->spinBox_DebounceTimer->value();
    devc->a2b_debounce_ms = ui->spinBox_A2bDebounce->value();

    devc->encoder_press_time_ms = ui->spinBox_EncoderPressTimer->value();

    // logical buttons
    for (int i = 0; i < m_logicButtonPtrList.size(); ++i) {
        m_logicButtonPtrList[i]->writeToConfig();
    }
}
