/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Anton Deguet
  Created on: 2013-08-24

  (C) Copyright 2013-2019 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/


// system include
#include <iostream>

// Qt include
#include <QString>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QScrollBar>
#include <QCloseEvent>
#include <QCoreApplication>

// cisst
#include <cisstMultiTask/mtsInterfaceRequired.h>
#include <cisstParameterTypes/prmPositionJointGet.h>

#include <sawIntuitiveResearchKit/mtsIntuitiveResearchKitArmQtWidget.h>


CMN_IMPLEMENT_SERVICES_DERIVED_ONEARG(mtsIntuitiveResearchKitArmQtWidget, mtsComponent, std::string);

mtsIntuitiveResearchKitArmQtWidget::mtsIntuitiveResearchKitArmQtWidget(const std::string & componentName, double periodInSeconds):
    mtsComponent(componentName),
    TimerPeriodInMilliseconds(periodInSeconds * 1000),
    DirectControl(false),
    LogEnabled(false)
{
    QMMessage = new mtsMessageQtWidget();
    QPOState = new prmOperatingStateQtWidget();

    // Setup CISST Interface
    InterfaceRequired = AddInterfaceRequired("Manipulator");
    if (InterfaceRequired) {
        InterfaceRequired->AddEventHandlerWrite(&mtsIntuitiveResearchKitArmQtWidget::DesiredStateEventHandler,
                                                this, "DesiredState");
        InterfaceRequired->AddEventHandlerWrite(&mtsIntuitiveResearchKitArmQtWidget::CurrentStateEventHandler,
                                                this, "CurrentState");
        QMMessage->SetInterfaceRequired(InterfaceRequired);
        InterfaceRequired->AddFunction("configuration_js", Arm.configuration_js);
        QPOState->SetInterfaceRequired(InterfaceRequired);
        InterfaceRequired->AddFunction("measured_js", Arm.measured_js);
        InterfaceRequired->AddFunction("measured_cp", Arm.measured_cp);
        InterfaceRequired->AddFunction("measured_cf_body", Arm.measured_cf_body, MTS_OPTIONAL);
        InterfaceRequired->AddFunction("move_jp", Arm.move_jp, MTS_OPTIONAL);
        InterfaceRequired->AddFunction("SetDesiredState", Arm.SetDesiredState);
        InterfaceRequired->AddFunction("period_statistics", Arm.period_statistics);
    }
}

void mtsIntuitiveResearchKitArmQtWidget::Configure(const std::string &filename)
{
    CMN_LOG_CLASS_INIT_VERBOSE << "Configure: " << filename << std::endl;
}

void mtsIntuitiveResearchKitArmQtWidget::Startup(void)
{
    setupUi();
    startTimer(TimerPeriodInMilliseconds); // ms
    if (!LogEnabled) {
        QMMessage->hide();
    }
    if (!parent()) {
        show();
    }
}

void mtsIntuitiveResearchKitArmQtWidget::Cleanup(void)
{
    this->hide();
}

void mtsIntuitiveResearchKitArmQtWidget::closeEvent(QCloseEvent * event)
{
    int answer = QMessageBox::warning(this, tr("mtsIntuitiveResearchKitArmQtWidget"),
                                      tr("Do you really want to quit this application?"),
                                      QMessageBox::No | QMessageBox::Yes);
    if (answer == QMessageBox::Yes) {
        event->accept();
        QCoreApplication::exit();
    } else {
        event->ignore();
    }
}

void mtsIntuitiveResearchKitArmQtWidget::timerEvent(QTimerEvent * CMN_UNUSED(event))
{
    // make sure we should update the display
    if (this->isHidden()) {
        return;
    }

    mtsExecutionResult executionResult;

    executionResult = Arm.measured_js(StateJoint);
    if (executionResult) {
        if ((ConfigurationJoint.Name().size() != StateJoint.Name().size())
            && (Arm.configuration_js.IsValid())) {
            Arm.configuration_js(ConfigurationJoint);
            QSJWidget->SetConfiguration(ConfigurationJoint);
        }
        QSJWidget->SetValue(StateJoint);
    }

    executionResult = Arm.measured_cp(Position);
    if (executionResult) {
        QCPGWidget->SetValue(Position);
    }

    executionResult = Arm.measured_cf_body(Wrench);
    if (executionResult) {
        if (Wrench.Valid()) {
            QFTWidget->SetValue(Wrench.F(), Wrench.T(), Wrench.Timestamp());
        }
    }

    Arm.period_statistics(IntervalStatistics);
    QMIntervalStatistics->SetValue(IntervalStatistics);

    // for derived classes
    this->timerEventDerived();
}

void mtsIntuitiveResearchKitArmQtWidget::DesiredStateEventHandler(const std::string & state)
{
    emit SignalDesiredState(QString(state.c_str()));
}

void mtsIntuitiveResearchKitArmQtWidget::CurrentStateEventHandler(const std::string & state)
{
    emit SignalCurrentState(QString(state.c_str()));
}

void mtsIntuitiveResearchKitArmQtWidget::SlotLogEnabled(void)
{
    LogEnabled = QPBLog->isChecked();
    if (LogEnabled) {
        QMMessage->show();
    } else {
        QMMessage->hide();
    }
}

void mtsIntuitiveResearchKitArmQtWidget::SlotEnableDirectControl(bool toggle)
{
    DirectControl = toggle;
    QPBHome->setEnabled(toggle);
    if (toggle) {
        QPJSWidget->show();
        QPJSWidget->setEnabled(toggle);
        QPJSWidget->Reset();
    } else {
        QPJSWidget->hide();
    }
}

void mtsIntuitiveResearchKitArmQtWidget::SlotHome(void)
{
    Arm.SetDesiredState(std::string("READY"));
}

void mtsIntuitiveResearchKitArmQtWidget::SlotDesiredStateEventHandler(QString state)
{
    QLEDesiredState->setText(state);
}

void mtsIntuitiveResearchKitArmQtWidget::SlotCurrentStateEventHandler(QString state)
{
    QLECurrentState->setText(state);
}

void mtsIntuitiveResearchKitArmQtWidget::setupUi(void)
{
    MainLayout = new QVBoxLayout;
    MainLayout->setContentsMargins(2, 2, 2, 2);

    QGridLayout * topLayout = new QGridLayout;
    topLayout->setContentsMargins(2, 2, 2, 2);
    topLayout->setColumnStretch(1, 1);

    MainLayout->addLayout(topLayout);

    // timing
    QMIntervalStatistics = new mtsQtWidgetIntervalStatistics();
    topLayout->addWidget(QMIntervalStatistics, 0, 0);

    // joint state
    QSJWidget = new prmStateJointQtWidget();
    QSJWidget->setupUi();
    QSJWidget->SetPrismaticRevoluteFactors(1.0 / cmn_mm, cmn180_PI);
    topLayout->addWidget(QSJWidget, 0, 1);

    // 3D position
    QCPGWidget = new prmPositionCartesianGetQtWidget();
    QCPGWidget->SetPrismaticRevoluteFactors(1.0 / cmn_mm, cmn180_PI);
    topLayout->addWidget(QCPGWidget, 1, 0);

    // wrench
    QFTWidget = new vctForceTorqueQtWidget();
    topLayout->addWidget(QFTWidget, 1, 1);

    // state
    QHBoxLayout * stateLayout = new QHBoxLayout;
    stateLayout->setContentsMargins(2, 2, 2, 2);
    MainLayout->addLayout(stateLayout);

    // messages on/off
    QPBLog = new QPushButton("Messages");
    QPBLog->setCheckable(true);
    stateLayout->addWidget(QPBLog);
    QCBEnableDirectControl = new QCheckBox("Direct control");
    stateLayout->addWidget(QCBEnableDirectControl);
    QPBHome = new QPushButton("Home");
    stateLayout->addWidget(QPBHome);

    QLabel * label = new QLabel("Desired");
    stateLayout->addWidget(label);
    QLEDesiredState = new QLineEdit("");
    QLEDesiredState->setReadOnly(true);
    stateLayout->addWidget(QLEDesiredState);

    label = new QLabel("Current");
    stateLayout->addWidget(label);
    QLECurrentState = new QLineEdit("");
    QLECurrentState->setReadOnly(true);
    stateLayout->addWidget(QLECurrentState);

    // operating state
    QPOState->setupUi();
    stateLayout->addWidget(QPOState);

    // set joint goal
    QPJSWidget = new prmPositionJointSetQtWidget();
    QPJSWidget->setupUi();
    QPJSWidget->measured_js = &(Arm.measured_js);
    QPJSWidget->configuration_js = &(Arm.configuration_js);
    QPJSWidget->move_jp = &(Arm.move_jp);
    QPJSWidget->SetPrismaticRevoluteFactors(1.0 / cmn_mm, cmn180_PI);
    MainLayout->addWidget(QPJSWidget);

    // for derived classes
    this->setupUiDerived();

    // messages
    QMMessage->setupUi();
    MainLayout->addWidget(QMMessage);

    setLayout(MainLayout);
    setWindowTitle("Manipulator");
    resize(sizeHint());

    // setup Qt Connection
    connect(this, SIGNAL(SignalDesiredState(QString)),
            this, SLOT(SlotDesiredStateEventHandler(QString)));
    connect(this, SIGNAL(SignalCurrentState(QString)),
            this, SLOT(SlotCurrentStateEventHandler(QString)));
    connect(QPBLog, SIGNAL(clicked()),
            this, SLOT(SlotLogEnabled()));
    connect(QCBEnableDirectControl, SIGNAL(toggled(bool)),
            this, SLOT(SlotEnableDirectControl(bool)));
    connect(QPBHome, SIGNAL(clicked()),
            this, SLOT(SlotHome()));

    // set initial values
    QCBEnableDirectControl->setChecked(DirectControl);
    SlotEnableDirectControl(DirectControl);
}
