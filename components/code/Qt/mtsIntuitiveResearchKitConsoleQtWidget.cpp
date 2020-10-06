/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Anton Deguet, Zihan Chen
  Created on: 2013-05-17

  (C) Copyright 2013-2020 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

// system include
#include <iostream>

// cisst
#include <cisstBuildType.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>
#include <cisstMultiTask/mtsComponentViewer.h>
#include <cisstMultiTask/mtsManagerLocal.h>
#include <sawIntuitiveResearchKit/mtsIntuitiveResearchKit.h>
#include <sawIntuitiveResearchKit/sawIntuitiveResearchKitRevision.h>
#include <sawIntuitiveResearchKit/mtsIntuitiveResearchKitConsoleQtWidget.h>

#include <QMessageBox>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QPushButton>
#include <QScrollBar>
#include <QGroupBox>
#include <QTabWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QShortcut>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QRadioButton>
#include <QCheckBox>
#include <QApplication>

CMN_IMPLEMENT_SERVICES(mtsIntuitiveResearchKitConsoleQtWidget);

mtsIntuitiveResearchKitConsoleQtWidget::mtsIntuitiveResearchKitConsoleQtWidget(const std::string & componentName):
    mtsComponent(componentName)
{
    QMMessage = new mtsMessageQtWidget();

    mtsInterfaceRequired * interfaceRequired = AddInterfaceRequired("Main");
    if (interfaceRequired) {
        QMMessage->SetInterfaceRequired(interfaceRequired);
        interfaceRequired->AddFunction("PowerOff", Console.PowerOff);
        interfaceRequired->AddFunction("PowerOn", Console.PowerOn);
        interfaceRequired->AddFunction("Home", Console.Home);
        interfaceRequired->AddEventHandlerWrite(&mtsIntuitiveResearchKitConsoleQtWidget::ArmCurrentStateEventHandler,
                                                this, "ArmCurrentState");
        interfaceRequired->AddFunction("TeleopEnable", Console.TeleopEnable);
        interfaceRequired->AddFunction("SelectTeleopPSM", Console.SelectTeleopPSM);
        interfaceRequired->AddEventHandlerWrite(&mtsIntuitiveResearchKitConsoleQtWidget::TeleopPSMSelectedEventHandler,
                                                this, "TeleopPSMSelected");
        interfaceRequired->AddEventHandlerWrite(&mtsIntuitiveResearchKitConsoleQtWidget::TeleopPSMUnselectedEventHandler,
                                                this, "TeleopPSMUnselected");
        interfaceRequired->AddFunction("SetScale", Console.SetScale);
        interfaceRequired->AddEventHandlerWrite(&mtsIntuitiveResearchKitConsoleQtWidget::ScaleEventHandler,
                                                this, "Scale");
        interfaceRequired->AddFunction("SetVolume", Console.SetVolume);
        interfaceRequired->AddFunction("EmulateOperatorPresent", Console.EmulateOperatorPresent);
        interfaceRequired->AddFunction("EmulateClutch", Console.EmulateClutch);
        interfaceRequired->AddFunction("EmulateCamera", Console.EmulateCamera);
    }
    interfaceRequired = AddInterfaceRequired("OperatorPresent");
    if (interfaceRequired) {
        interfaceRequired->AddEventHandlerWrite(&mtsIntuitiveResearchKitConsoleQtWidget::OperatorPresentEventHandler,
                                                this, "Button");
    }
    interfaceRequired = AddInterfaceRequired("Clutch");
    if (interfaceRequired) {
        interfaceRequired->AddEventHandlerWrite(&mtsIntuitiveResearchKitConsoleQtWidget::ClutchEventHandler,
                                                this, "Button");
    }
    interfaceRequired = AddInterfaceRequired("Camera");
    if (interfaceRequired) {
        interfaceRequired->AddEventHandlerWrite(&mtsIntuitiveResearchKitConsoleQtWidget::CameraEventHandler,
                                                this, "Button");
    }
    setupUi();
}

void mtsIntuitiveResearchKitConsoleQtWidget::Configure(const std::string & filename)
{
    CMN_LOG_CLASS_INIT_VERBOSE << "Configure: " << filename << std::endl;
}

void mtsIntuitiveResearchKitConsoleQtWidget::Startup(void)
{
    CMN_LOG_CLASS_INIT_VERBOSE << "mtsIntuitiveResearchKitConsoleQtWidget::Startup" << std::endl;
    if (!parent()) {
        show();
    }

    // write warning to cerr if not compiled in Release mode
    if (std::string(CISST_BUILD_TYPE) != "Release") {
        std::string message;
        message.append("Warning:\n");
        message.append(" It seems that \"cisst\" has not been compiled in\n");
        message.append(" Release mode.  Make sure your CMake configuration\n");
        message.append(" or catkin profile is configured to compile in\n");
        message.append(" Release mode for better performance and stability");

        QMessageBox * msgBox = new QMessageBox(this);
        msgBox->setAttribute(Qt::WA_DeleteOnClose);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->setWindowTitle("Warning");
        msgBox->setText(message.c_str());
        msgBox->setModal(true);
        msgBox->show();
    }
}

void mtsIntuitiveResearchKitConsoleQtWidget::Cleanup(void)
{
    this->hide();
    CMN_LOG_CLASS_INIT_VERBOSE << "mtsIntuitiveResearchKitConsoleQtWidget::Cleanup" << std::endl;
}

void mtsIntuitiveResearchKitConsoleQtWidget::HasTeleOp(const bool & hasTeleOp)
{
    QPBTeleopStart->setEnabled(hasTeleOp);
    QPBTeleopStop->setEnabled(hasTeleOp);
    QSBScale->setEnabled(hasTeleOp);
}

void mtsIntuitiveResearchKitConsoleQtWidget::closeEvent(QCloseEvent * event)
{
    int answer = QMessageBox::warning(this, tr("mtsIntuitiveResearchKitConsoleQtWidget"),
                                      tr("Do you really want to quit this application?"),
                                      QMessageBox::No | QMessageBox::Yes);
    if (answer == QMessageBox::Yes) {
        event->accept();
        this->hide();
        // send clean power off message and wait a bit
        Console.PowerOff();
        osaSleep(2.0 * cmn_s);
        QCoreApplication::exit();
    } else {
        event->ignore();
    }
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotPowerOff(void)
{
    Console.PowerOff();
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotPowerOn(void)
{
    Console.PowerOn();
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotHome(void)
{
    Console.Home();
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotArmCurrentStateEventHandler(PairStringType armState)
{
    const QString arm = armState.first;
    auto iter = ArmButtons.find(arm);
    QPushButton * button;
    // insert new arm if needed
    if (iter == ArmButtons.end()) {
        button = new QPushButton(arm);
        QVBArms->addWidget(button);
        ArmButtons[arm] = button;
        connect(button, &QPushButton::clicked,
                [ = ] { FocusArmButton(armState.first); });
    } else {
        button = iter->second;
    }
    // color code state
    QString state = armState.second;
    if (state == "ENABLED") {
        button->setStyleSheet("QPushButton { background-color: rgb(50, 255, 50); border: none }");
    } else if (state == "FAULT") {
        button->setStyleSheet("QPushButton { background-color: rgb(255, 100, 100); border: none }");
    } else {
        button->setStyleSheet("QPushButton { background-color: none; border: none }");
    }
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotTeleopStart(void)
{
    Console.TeleopEnable(true);
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotTeleopStop(void)
{
    Console.TeleopEnable(false);
}

void mtsIntuitiveResearchKitConsoleQtWidget::GetTeleopButtonCheck(const PairStringType & pair,
                                                                  QPushButton * & button,
                                                                  QCheckBox * & check)
{
    const QString teleop = pair.first + "-" + pair.second;
    auto iter = TeleopButtons.find(teleop);
    // insert new teleop if needed
    if (iter == TeleopButtons.end()) {
        QHBoxLayout * buttonsLayout = new QHBoxLayout;
        button = new QPushButton(teleop);
        buttonsLayout->addWidget(button);
        buttonsLayout->addStretch();
        check = new QCheckBox("");
        buttonsLayout->addWidget(check);
        QVBTeleops->addLayout(buttonsLayout);
        TeleopButtons[teleop] = std::pair<QPushButton *, QCheckBox *>(button, check);
        connect(button, &QPushButton::clicked,
                [ = ] { FocusTeleopButton(teleop); });
        connect(check, &QCheckBox::toggled,
                [ = ](bool checked) {
                    if (checked) {
                        SelectTeleopCheck(pair);
                    } else {
                        UnselectTeleopCheck(pair);
                    }
                });
    } else {
        button = iter->second.first;
        check = iter->second.second;
    }
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotTeleopPSMSelectedEventHandler(PairStringType selected)
{
    QPushButton * button;
    QCheckBox * check;
    GetTeleopButtonCheck(selected, button, check);
    button->setStyleSheet("QPushButton { border: none }");
    check->blockSignals(true);
    check->setChecked(true);
    check->blockSignals(false);
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotTeleopPSMUnselectedEventHandler(PairStringType unselected)
{
    QPushButton * button;
    QCheckBox * check;
    GetTeleopButtonCheck(unselected, button, check);
    button->setStyleSheet("QPushButton { border: none; color: palette(mid) }");
    check->blockSignals(true);
    check->setChecked(false);
    check->blockSignals(false);
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotSetScale(double scale)
{
    Console.SetScale(scale);
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotSetVolume(void)
{
    double volume01 = static_cast<double>(QSVolume->value()) / 100.0;
    Console.SetVolume(volume01);
}

void mtsIntuitiveResearchKitConsoleQtWidget::setupUi(void)
{
    QHBoxLayout * mainLayout = new QHBoxLayout;

    QWidget * buttonsWidget = new QWidget();
    QVBoxLayout * boxLayout = new QVBoxLayout();
    boxLayout->setContentsMargins(0, 0, 0, 0);
    buttonsWidget->setLayout(boxLayout);

    QGroupBox * armsBox = new QGroupBox("Arms");
    boxLayout->addWidget(armsBox);
    QVBoxLayout * armsLayout = new QVBoxLayout();
    armsLayout->setContentsMargins(2, 2, 2, 2);
    armsBox->setLayout(armsLayout);
    QPBPowerOff = new QPushButton("Power Off");
    QPBPowerOff->setToolTip("ctrl + O");
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this, SLOT(SlotPowerOff()));
    armsLayout->addWidget(QPBPowerOff);
    QPBPowerOn = new QPushButton("Power On");
    QPBPowerOn->setToolTip("ctrl + P");
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_P), this, SLOT(SlotPowerOn()));
    armsLayout->addWidget(QPBPowerOn);
    QPBHome = new QPushButton("Home");
    QPBHome->setToolTip("ctrl + H");
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_H), this, SLOT(SlotHome()));
    armsLayout->addWidget(QPBHome);
    // arm buttons
    QVBArms = new QVBoxLayout();
    armsLayout->addLayout(QVBArms);

    QGroupBox * teleopBox = new QGroupBox("Tele operation");
    boxLayout->addWidget(teleopBox);
    QVBoxLayout * teleopLayout = new QVBoxLayout();
    teleopLayout->setContentsMargins(2, 2, 2, 2);
    teleopBox->setLayout(teleopLayout);
    QPBTeleopStart = new QPushButton("Start");
    QPBTeleopStart->setToolTip("ctrl + T");
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_T), this, SLOT(SlotTeleopStart()));
    teleopLayout->addWidget(QPBTeleopStart);
    QPBTeleopStop = new QPushButton("Stop");
    QPBTeleopStop->setToolTip("ctrl + S");
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, SLOT(SlotTeleopStop()));
    teleopLayout->addWidget(QPBTeleopStop);
    QSBScale = new QDoubleSpinBox();
    QSBScale->setRange(0.1, 1.0);
    QSBScale->setSingleStep(0.1);
    QSBScale->setPrefix("scale ");
    QSBScale->setValue(mtsIntuitiveResearchKit::TeleOperationPSM::Scale);
    teleopLayout->addWidget(QSBScale);
    QVBTeleops = new QVBoxLayout();
    teleopLayout->addLayout(QVBTeleops);

    QGroupBox * inputsBox = new QGroupBox("Inputs");
    boxLayout->addWidget(inputsBox);
    QVBoxLayout * inputsLayout = new QVBoxLayout();
    inputsLayout->setContentsMargins(2, 2, 2, 2);
    inputsBox->setLayout(inputsLayout);
    QRBOperatorPresent = new QRadioButton("Operator");
    QRBOperatorPresent->setAutoExclusive(false);
    QRBOperatorPresent->setChecked(false);
    QRBOperatorPresent->setEnabled(false);
    inputsLayout->addWidget(QRBOperatorPresent);
    QRBClutch = new QRadioButton("Clutch");
    QRBClutch->setAutoExclusive(false);
    QRBClutch->setChecked(false);
    QRBClutch->setEnabled(false);
    inputsLayout->addWidget(QRBClutch);
    QRBCamera = new QRadioButton("Camera");
    QRBCamera->setAutoExclusive(false);
    QRBCamera->setChecked(false);
    QRBCamera->setEnabled(false);
    inputsLayout->addWidget(QRBCamera);

    QGroupBox * audioBox = new QGroupBox("Audio");
    boxLayout->addWidget(audioBox);
    QVBoxLayout * audioLayout = new QVBoxLayout();
    audioLayout->setContentsMargins(2, 2, 2, 2);
    audioBox->setLayout(audioLayout);
    QSVolume = new QSlider(Qt::Horizontal);
    QSVolume->setRange(0, 100);
    QSVolume->setValue(50);
    audioLayout->addWidget(QSVolume);

    boxLayout->addStretch(100);
    buttonsWidget->setFixedWidth(buttonsWidget->sizeHint().width());
    mainLayout->addWidget(buttonsWidget);

    QCBEnableDirectControl = new QCheckBox("Direct control");
    QCBEnableDirectControl->setToolTip("Allows to emulate console events with buttons");
    boxLayout->addWidget(QCBEnableDirectControl);

    QPBComponentViewer = new QPushButton("Component\nViewer");
    QPBComponentViewer->setToolTip("Starts uDrawGraph (must be in system path)");
    boxLayout->addWidget(QPBComponentViewer);

    QLabel * labelLogo = new QLabel("");
    labelLogo->setPixmap(QPixmap(":/dVRK.png").scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    boxLayout->addWidget(labelLogo);

    QSplitter * tabWidgetAndMessages = new QSplitter();
    tabWidgetAndMessages->setOrientation(Qt::Vertical);

    QTWidgets = new QTabWidget();
    tabWidgetAndMessages->addWidget(QTWidgets);

    QMMessage->setupUi();
    tabWidgetAndMessages->addWidget(QMMessage);

    mainLayout->addWidget(tabWidgetAndMessages);
    setLayout(mainLayout);

    std::string title = "dVRK ";
    title.append(sawIntuitiveResearchKit_VERSION);
    title.append(" / cisst ");
    title.append(CISST_VERSION);
    setWindowTitle(title.c_str());
    resize(sizeHint());

    // buttons
    connect(QPBPowerOff, SIGNAL(clicked()),
            this, SLOT(SlotPowerOff()));
    connect(QPBPowerOn, SIGNAL(clicked()),
            this, SLOT(SlotPowerOn()));
    connect(QPBHome, SIGNAL(clicked()),
            this, SLOT(SlotHome()));
    qRegisterMetaType<PairStringType>("PairStringType");
    connect(this, SIGNAL(SignalArmCurrentState(PairStringType)),
            this, SLOT(SlotArmCurrentStateEventHandler(PairStringType)));
    connect(QPBTeleopStart, SIGNAL(clicked()),
            this, SLOT(SlotTeleopStart()));
    connect(QPBTeleopStop, SIGNAL(clicked()),
            this, SLOT(SlotTeleopStop()));
    connect(this, SIGNAL(SignalTeleopPSMSelected(PairStringType)),
            this, SLOT(SlotTeleopPSMSelectedEventHandler(PairStringType)));
    connect(this, SIGNAL(SignalTeleopPSMUnselected(PairStringType)),
            this, SLOT(SlotTeleopPSMUnselectedEventHandler(PairStringType)));
    connect(QSBScale, SIGNAL(valueChanged(double)),
            this, SLOT(SlotSetScale(double)));
    connect(this, SIGNAL(SignalScale(double)),
            this, SLOT(SlotScaleEventHandler(double)));
    connect(this, SIGNAL(SignalOperatorPresent(bool)),
            this, SLOT(SlotOperatorPresentEventHandler(bool)));
    connect(this, SIGNAL(SignalClutch(bool)),
            this, SLOT(SlotClutchEventHandler(bool)));
    connect(this, SIGNAL(SignalCamera(bool)),
            this, SLOT(SlotCameraEventHandler(bool)));
    connect(QSVolume, SIGNAL(sliderReleased()),
            this, SLOT(SlotSetVolume()));
    connect(QCBEnableDirectControl, SIGNAL(toggled(bool)),
            this, SLOT(SlotEnableDirectControl(bool)));
    connect(QRBOperatorPresent, SIGNAL(clicked(bool)),
            this, SLOT(SlotEmulateOperatorPresent(bool)));
    connect(QRBClutch, SIGNAL(clicked(bool)),
            this, SLOT(SlotEmulateClutch(bool)));
    connect(QRBCamera, SIGNAL(clicked(bool)),
            this, SLOT(SlotEmulateCamera(bool)));
    connect(QPBComponentViewer, SIGNAL(clicked()),
            this, SLOT(SlotComponentViewer()));

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
}

void mtsIntuitiveResearchKitConsoleQtWidget::ArmCurrentStateEventHandler(const prmKeyValue & armState)
{
    PairStringType currentState;
    currentState.first = armState.Key.c_str();
    currentState.second = armState.Value.c_str();
    emit SignalArmCurrentState(currentState);
}

void mtsIntuitiveResearchKitConsoleQtWidget::TeleopPSMSelectedEventHandler(const prmKeyValue & selected)
{
    PairStringType currentSelected;
    currentSelected.first = selected.Key.c_str();
    currentSelected.second = selected.Value.c_str();
    emit SignalTeleopPSMSelected(currentSelected);
}

void mtsIntuitiveResearchKitConsoleQtWidget::TeleopPSMUnselectedEventHandler(const prmKeyValue & unselected)
{
    PairStringType currentUnselected;
    currentUnselected.first = unselected.Key.c_str();
    currentUnselected.second = unselected.Value.c_str();
    emit SignalTeleopPSMUnselected(currentUnselected);
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotScaleEventHandler(double scale)
{
    QSBScale->setValue(scale);
}

void mtsIntuitiveResearchKitConsoleQtWidget::ScaleEventHandler(const double & scale)
{
    emit SignalScale(scale);
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotOperatorPresentEventHandler(bool operatorPresent)
{
    QRBOperatorPresent->setChecked(operatorPresent);
    QApplication::beep();
}

void mtsIntuitiveResearchKitConsoleQtWidget::OperatorPresentEventHandler(const prmEventButton & button)
{
    if (button.Type() == prmEventButton::PRESSED) {
        emit SignalOperatorPresent(true);
    } else {
        emit SignalOperatorPresent(false);
    }
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotClutchEventHandler(bool clutch)
{
    QRBClutch->setChecked(clutch);
}

void mtsIntuitiveResearchKitConsoleQtWidget::ClutchEventHandler(const prmEventButton & button)
{
    if (button.Type() == prmEventButton::PRESSED) {
        emit SignalClutch(true);
    } else {
        emit SignalClutch(false);
    }
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotCameraEventHandler(bool camera)
{
    QRBCamera->setChecked(camera);
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotEnableDirectControl(bool toggle)
{
    if (toggle) {
        int answer = QMessageBox::warning(this, tr("mtsIntuitiveResearchKitConsoleQtWidget"),
                                          tr("Mixing real and emulated console events can lead to inconsistent states.\nAre you sure you want to continue?"),
                                          QMessageBox::No | QMessageBox::Yes);
        if (answer == QMessageBox::No) {
            return;
        }
    }
    QRBOperatorPresent->setEnabled(toggle);
    QRBClutch->setEnabled(toggle);
    QRBCamera->setEnabled(toggle);
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotEmulateOperatorPresent(bool toggle)
{
    prmEventButton event;
    if (toggle) {
        event.SetType(prmEventButton::PRESSED);
    } else {
        event.SetType(prmEventButton::RELEASED);
    }
    Console.EmulateOperatorPresent(event);
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotEmulateClutch(bool toggle)
{
    prmEventButton event;
    if (toggle) {
        event.SetType(prmEventButton::PRESSED);
    } else {
        event.SetType(prmEventButton::RELEASED);
    }
    Console.EmulateClutch(event);
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotEmulateCamera(bool toggle)
{
    prmEventButton event;
    if (toggle) {
        event.SetType(prmEventButton::PRESSED);
    } else {
        event.SetType(prmEventButton::RELEASED);
    }
    Console.EmulateCamera(event);
}

void mtsIntuitiveResearchKitConsoleQtWidget::SlotComponentViewer(void)
{
    QPBComponentViewer->setEnabled(false);
    std::cerr << "Now trying to launch uDrawGraph." << std::endl
              << "uDrawGraph needs to be installed in your path and the variable UDG_HOME set." << std::endl
              << "See http://www.informatik.uni-bremen.de/uDrawGraph/en/download/download.html" << std::endl;
    mtsManagerLocal * componentManager = mtsManagerLocal::GetInstance();
    mtsComponentViewer * componentViewer = new mtsComponentViewer("ComponentViewer");
    componentManager->AddComponent(componentViewer);
    osaSleep(0.2 * cmn_s);
    componentViewer->Create();
    osaSleep(0.2 * cmn_s);
    componentViewer->Start();
}

void mtsIntuitiveResearchKitConsoleQtWidget::FocusArmButton(const QString & armName)
{
    // determine which tab to search
    QTabWidget * subTab = QTWidgets->findChild<QTabWidget *>(QString("Arms"));
    if (subTab) {
        QTWidgets->setCurrentWidget(subTab);
    } else {
        subTab = QTWidgets;
    }

    // now find the arm widget
    QWidget * child = subTab->findChild<QWidget *>(armName);
    if (child) {
        subTab->setCurrentWidget(child);
    } else {
        std::cerr << CMN_LOG_DETAILS << " can't find arm nor Arms tab widget for \""
                  << armName.toStdString() << "\", did you set the widget name with setObjectName?" << std::endl;
    }
}

void mtsIntuitiveResearchKitConsoleQtWidget::FocusTeleopButton(const QString & teleop)
{
    // determine which tab to search
    QTabWidget * subTab = QTWidgets->findChild<QTabWidget *>(QString("Teleops"));
    if (subTab) {
        QTWidgets->setCurrentWidget(subTab);
    } else {
        subTab = QTWidgets;
    }

    // now find the arm widget
    QWidget * child = subTab->findChild<QWidget *>(teleop);
    if (child) {
        subTab->setCurrentWidget(child);
    } else {
        std::cerr << CMN_LOG_DETAILS << " can't find teleop widget for \""
                  << teleop.toStdString() << "\", did you set the widget name with setObjectName?" << std::endl;
    }
}

void mtsIntuitiveResearchKitConsoleQtWidget::SelectTeleopCheck(const PairStringType & pair)
{
    Console.SelectTeleopPSM(prmKeyValue(pair.first.toStdString(),
                                        pair.second.toStdString()));
}

void mtsIntuitiveResearchKitConsoleQtWidget::UnselectTeleopCheck(const PairStringType & pair)
{
    Console.SelectTeleopPSM(prmKeyValue(pair.first.toStdString(),
                                        std::string()));
}

void mtsIntuitiveResearchKitConsoleQtWidget::CameraEventHandler(const prmEventButton & button)
{
    if (button.Type() == prmEventButton::PRESSED) {
        emit SignalCamera(true);
    } else {
        emit SignalCamera(false);
    }
}
