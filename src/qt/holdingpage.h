// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_HOLDINGPAGE_H
#define BITCOIN_QT_HOLDINGPAGE_H

#include "amount.h"
#include "uint256.h"

#include <QWidget>
#include <QTableWidgetItem>

class ClientModel;
class TransactionFilterProxy;
class TxViewDelegate2;
class WalletModel;

namespace Ui
{
class HoldingPage;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Holding page widget */
class HoldingPage : public QWidget
{
    Q_OBJECT

public:
    explicit HoldingPage(QWidget* parent = 0);
    ~HoldingPage();

    void setClientModel(ClientModel* clientModel);
    void setWalletModel(WalletModel* walletModel);

public slots:
	void setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& holdingBalance, const CAmount& immatureBalance, const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);
    void setHoldingBalance(const CAmount& holdingBalance);
	void pushButtonLevel1Clicked();
	void pushButtonLevel2Clicked();
	void pushButtonLevel3Clicked();
	void pushButtonLevel4Clicked();
	void pushButtonLevel5Clicked();

public Q_SLOTS:
    void updateHoldingList();
    void reloadHoldingTimes();

private:
    QTimer* timer;
    Ui::HoldingPage* ui;
    ClientModel* clientModel;
    WalletModel* walletModel;
    CAmount currentHoldingBalance;
    int nDisplayUnit;
	int nInsertedHeight;
	std::map<uint32_t, QTableWidgetItem*> mapRemainItems;
	std::map<uint32_t, int> mapEndBlocks;

private slots:
    void updateDisplayUnit();
};

#endif // BITCOIN_QT_HOLDINGPAGE_H
