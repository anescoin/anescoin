// Copyright (c) 2011-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Anes developers
// Copyright (c) 2015-2017 The PIVX developers 
// Copyright (c) 2015-2017 The ANES developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "holdingpage.h"
#include "ui_holdingpage.h"

#include "bitcoinunits.h"
#include "clientmodel.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "init.h"
#include "optionsmodel.h"
#include "transactionfilterproxy.h"
#include "transactiontablemodel.h"
#include "walletmodel.h"
#include "main.h"
#include "utiltime.h"

#include <QPainter>
#include <QSettings>
#include <QTimer>

#define DECORATION_SIZE 48
#define ICON_OFFSET 16

#include "holdingpage.moc"

HoldingPage::HoldingPage(QWidget* parent) : QWidget(parent),
                                              ui(new Ui::HoldingPage),
                                              clientModel(0),
                                              walletModel(0),
                                              currentHoldingBalance(-1),
											  nInsertedHeight(0)
{
    nDisplayUnit = 0; // just make sure it's not unitialized
    ui->setupUi(this);

    int columnToAmountWidth = 150;
    int columnWaitDaysWidth = 130;
    int columnRemainDaysWidth = 130;

    ui->tableWidgetHolding->setColumnWidth(0, columnToAmountWidth);
    ui->tableWidgetHolding->setColumnWidth(1, columnWaitDaysWidth);
    ui->tableWidgetHolding->setColumnWidth(2, columnRemainDaysWidth);
	ui->tableWidgetHolding->sortItems(2, Qt::AscendingOrder);

	std::string baseStr = BitcoinUnits::format(nDisplayUnit, Params().HoldingValue(), false, BitcoinUnits::separatorAlways).toStdString();
	QString lv1;
	lv1.sprintf("[%s Days : ROI %s%% year] %s -> %s", QString::number(Params().GetHoldingWaitBlock(1) / (float)Params().BlocksPerDay()).toStdString().c_str(), QString::number(Params().GetHoldingMultiply(1)*100).toStdString().c_str(), baseStr.c_str(), BitcoinUnits::format(nDisplayUnit, Params().GetHoldingOutput(1, Params().HoldingValue()), false, BitcoinUnits::separatorAlways).toStdString().c_str());
	ui->pushButtonLevel1->setText(lv1);
	QString lv2;
	lv2.sprintf("[%s Days : ROI %s%% year] %s -> %s", QString::number(Params().GetHoldingWaitBlock(2) / (float)Params().BlocksPerDay()).toStdString().c_str(), QString::number(Params().GetHoldingMultiply(2)*100).toStdString().c_str(), baseStr.c_str(), BitcoinUnits::format(nDisplayUnit, Params().GetHoldingOutput(2, Params().HoldingValue()), false, BitcoinUnits::separatorAlways).toStdString().c_str());
	ui->pushButtonLevel2->setText(lv2);
	QString lv3;
	lv3.sprintf("[%s Days : ROI %s%% year] %s -> %s", QString::number(Params().GetHoldingWaitBlock(3) / (float)Params().BlocksPerDay()).toStdString().c_str(), QString::number(Params().GetHoldingMultiply(3)*100).toStdString().c_str(), baseStr.c_str(), BitcoinUnits::format(nDisplayUnit, Params().GetHoldingOutput(3, Params().HoldingValue()), false, BitcoinUnits::separatorAlways).toStdString().c_str());
	ui->pushButtonLevel3->setText(lv3);
	QString lv4;
	lv4.sprintf("[%s Days : ROI %s%% year] %s -> %s", QString::number(Params().GetHoldingWaitBlock(4) / (float)Params().BlocksPerDay()).toStdString().c_str(), QString::number(Params().GetHoldingMultiply(4)*100).toStdString().c_str(), baseStr.c_str(), BitcoinUnits::format(nDisplayUnit, Params().GetHoldingOutput(4, Params().HoldingValue()), false, BitcoinUnits::separatorAlways).toStdString().c_str());
	ui->pushButtonLevel4->setText(lv4);
	QString lv5;
	lv5.sprintf("[%s Days : ROI %s%% year] %s -> %s", QString::number(Params().GetHoldingWaitBlock(5) / (float)Params().BlocksPerDay()).toStdString().c_str(), QString::number(Params().GetHoldingMultiply(5)*100).toStdString().c_str(), baseStr.c_str(), BitcoinUnits::format(nDisplayUnit, Params().GetHoldingOutput(5, Params().HoldingValue()), false, BitcoinUnits::separatorAlways).toStdString().c_str());
	ui->pushButtonLevel5->setText(lv5);
	
	connect(ui->pushButtonLevel1, SIGNAL(clicked()), this, SLOT(pushButtonLevel1Clicked()));
    connect(ui->pushButtonLevel2, SIGNAL(clicked()), this, SLOT(pushButtonLevel2Clicked()));
    connect(ui->pushButtonLevel3, SIGNAL(clicked()), this, SLOT(pushButtonLevel3Clicked()));
    connect(ui->pushButtonLevel4, SIGNAL(clicked()), this, SLOT(pushButtonLevel4Clicked()));
    connect(ui->pushButtonLevel5, SIGNAL(clicked()), this, SLOT(pushButtonLevel5Clicked()));

	timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(reloadHoldingTimes()));
    timer->start(1000);
}

HoldingPage::~HoldingPage()
{
    delete ui;
}

void HoldingPage::setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& holdingBalance, const CAmount& immatureBalance, const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance)
{
	setHoldingBalance(holdingBalance);
}

void HoldingPage::setHoldingBalance(const CAmount& holdingBalance)
{
    currentHoldingBalance = holdingBalance;
    
    ui->labelHolding->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, holdingBalance, false, BitcoinUnits::separatorAlways));

	updateHoldingList();
}

void HoldingPage::setClientModel(ClientModel* model)
{
    this->clientModel = model;
}

void HoldingPage::setWalletModel(WalletModel* model)
{
    this->walletModel = model;
    if (model && model->getOptionsModel()) {
        // Keep up to date with wallet
        setHoldingBalance(model->getHoldingBalance());
        connect(model, SIGNAL(balanceChanged(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)), this, SLOT(setBalance(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
    }

    // update the display unit, to not use the default ("ANES")
    updateDisplayUnit();
	updateHoldingList();
}

void HoldingPage::updateDisplayUnit()
{
    if (walletModel && walletModel->getOptionsModel()) {
        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();
        if (currentHoldingBalance != -1)
			 setHoldingBalance(currentHoldingBalance);
		updateHoldingList();
    }
}

void HoldingPage::updateHoldingList()
{
    ui->tableWidgetHolding->setSortingEnabled(false);
	if(walletModel) {
		int nLastHeight = nInsertedHeight;
		CWallet *wallet = walletModel->getWallet();
		LOCK2(cs_main, wallet->cs_wallet);
		for (std::map<uint256, CWalletTx>::iterator it = wallet->mapWallet.begin(); it != wallet->mapWallet.end(); ++it) {
			CWalletTx &wtx = it->second;
			if(wtx.GetDepthInMainChain() < 0) {
				continue;
			}
			txnouttype type;
			vector<CTxDestination> addresses;
			int nRequired;
			if (ExtractDestinations(wtx.vout[0].scriptPubKey, type, addresses, nRequired) && IsMine(*wallet, addresses[0])) {
				bool IsHolding = (strcmp(GetTxnOutputType(type), "holding") == 0);
				if(IsHolding) {
					BlockMap::iterator mi = mapBlockIndex.find(wtx.hashBlock);
					int nBlockHeight = 0;
					if (mi != mapBlockIndex.end()) {
						nBlockHeight = mi->second->nHeight;
					} else {
						nBlockHeight = chainActive.Height();
					}


					const CScript& script1 = wtx.vout[0].scriptPubKey;
					CScript::const_iterator pc1 = script1.begin();
					opcodetype opcode1;
					vector<unsigned char> vch1;
					if(!script1.GetOp(pc1, opcode1, vch1)) {
						continue;
					}
					CScriptNum WaitBlock(vch1, false, 5);
					int nWaitBlock = WaitBlock.getint();

					float fRemain = nWaitBlock - (chainActive.Height() - nBlockHeight);
					std::map<uint32_t, QTableWidgetItem*>::iterator iter = mapRemainItems.find(it->first.GetCompact());
					if(iter != mapRemainItems.end()) {
						std::map<uint32_t, int>::iterator iter_end = mapEndBlocks.find(iter->first);
						if(iter_end != mapEndBlocks.end()) {
							if(fRemain >= 0) {
								iter->second->setText(QString::fromStdString(DurationToDHMS(fRemain / (float)Params().BlocksPerDay() * 86400.f)));
							} else {
								iter->second->setText(QString("Paid"));
							}
						}
					} else {
						QTableWidgetItem* remainDaysItem;
						if(fRemain >= 0) {
							remainDaysItem = new QTableWidgetItem(QString::fromStdString(DurationToDHMS(fRemain / (float)Params().BlocksPerDay() * 86400.f)));
						} else {
							remainDaysItem = new QTableWidgetItem(QString("Paid"));
						}
						QTableWidgetItem* toAmountItem = new QTableWidgetItem(BitcoinUnits::format(nDisplayUnit, wtx.vout[0].nValue, false, BitcoinUnits::separatorAlways));
						GUIUtil::DHMSTableWidgetItem* waitDaysItem = new GUIUtil::DHMSTableWidgetItem((float)nWaitBlock / (float)Params().BlocksPerDay() * 86400.f);
						
						ui->tableWidgetHolding->insertRow(0);
						ui->tableWidgetHolding->setItem(0, 0, toAmountItem);
						ui->tableWidgetHolding->setItem(0, 1, waitDaysItem);
						ui->tableWidgetHolding->setItem(0, 2, remainDaysItem);

						uint32_t compact = it->first.GetCompact();
						mapRemainItems.insert(std::make_pair(compact, remainDaysItem));
						mapEndBlocks.insert(std::make_pair(compact, nWaitBlock + nBlockHeight));
					}
					if(nLastHeight < nBlockHeight) {
						nLastHeight = nBlockHeight;
					}
				}
			}
		}
		nInsertedHeight = nLastHeight;
	}
    ui->tableWidgetHolding->setSortingEnabled(true);
}

void HoldingPage::reloadHoldingTimes()
{
	for(std::map<uint32_t, QTableWidgetItem*>::iterator iter = mapRemainItems.begin(); iter != mapRemainItems.end(); ++iter) {
		std::map<uint32_t, int>::iterator iter_end = mapEndBlocks.find(iter->first);
		if(iter_end != mapEndBlocks.end()) {
			float fRemain = iter_end->second - chainActive.Height();
			if(fRemain >= 0) {
				iter->second->setText(QString::fromStdString(DurationToDHMS(fRemain / (float)Params().BlocksPerDay() * 86400.f)));
			} else {
				iter->second->setText(QString("Paid"));
			}
		}
	}
}

void HoldingPage::pushButtonLevel1Clicked() {
	try {
		HoldingMoney("holding1", 1);
		updateHoldingList();
	}
	catch (const char* err) {
		QMessageBox::warning(this, windowTitle(), err, QMessageBox::Ok, QMessageBox::Ok);
	}
}

void HoldingPage::pushButtonLevel2Clicked() {
	try {
		HoldingMoney("holding2", 2);
		updateHoldingList();
	}
	catch (const char* err) {
		QMessageBox::warning(this, windowTitle(), err, QMessageBox::Ok, QMessageBox::Ok);
	}
}

void HoldingPage::pushButtonLevel3Clicked() {
	try {
		HoldingMoney("holding3", 3);
		updateHoldingList();
	}
	catch (const char* err) {
		QMessageBox::warning(this, windowTitle(), err, QMessageBox::Ok, QMessageBox::Ok);
	}
}

void HoldingPage::pushButtonLevel4Clicked() {
	try {
		HoldingMoney("holding4", 4);
		updateHoldingList();
	}
	catch (const char* err) {
		QMessageBox::warning(this, windowTitle(), err, QMessageBox::Ok, QMessageBox::Ok);
	}
}

void HoldingPage::pushButtonLevel5Clicked() {
	try {
		HoldingMoney("holding5", 5);
		updateHoldingList();
	}
	catch (const char* err) {
		QMessageBox::warning(this, windowTitle(), err, QMessageBox::Ok, QMessageBox::Ok);
	}
}
