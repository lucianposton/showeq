/*
 * netdiag.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 *
 */

#include <qpushbutton.h>
#include <qlcdnumber.h>
#include <qaccel.h>

#include "main.h"
#include "netdiag.h"

NetDiag::NetDiag(EQPacket* packet, QWidget* parent, const char* name = NULL)
  : QFrame(parent, name),
    m_packet(packet),
    m_playbackSpeed(NULL)
{
  QGridLayout* tmpGrid = new QGridLayout(this, 6, 9);
  tmpGrid->addColSpacing(3, 5);
  tmpGrid->addColSpacing(6, 5);
  tmpGrid->addRowSpacing(1, 5);
  tmpGrid->addRowSpacing(3, 5);
  tmpGrid->addRowSpacing(6, 5);
  tmpGrid->addRowSpacing(8, 5);

  // get preferences
  QString section = "NetDiag";

  // set caption
  QFrame::setCaption(pSEQPrefs->getPrefString("Caption", section,
					      "ShowEQ - Network Diagnostics"));

  int row = 0;
  int col = 0;

  // create a widgets to display the info
  // packet throughput
  tmpGrid->addWidget(new QLabel("Packets ", this), row, col++);
  tmpGrid->addWidget(new QLabel("Total: ", this), row, col++);
  m_packetTotal = new QLabel(this, "count");
  tmpGrid->addWidget(m_packetTotal, row, col++);
  col++;
  tmpGrid->addWidget(new QLabel("Recent: ", this), row, col++);
  m_packetRecent = new QLabel(this, "recent");
  tmpGrid->addWidget(m_packetRecent, row, col++);
  col++;
  tmpGrid->addWidget(new QLabel("Rate: ", this), row, col++);
  m_packetAvg = new QLabel(this, "avg");
  tmpGrid->addWidget(m_packetAvg, row, col++);
  resetPacket(m_packet->packetCount());
  row++; row++; col = 0;

  // network status
  tmpGrid->addWidget(new QLabel("Status ", this), row, col++);
  tmpGrid->addWidget(new QLabel("Cached: ", this), row, col++);
  QLabel* cache = new QLabel(this, "cached");
  cache->setNum(m_packet->currentCacheSize());
  tmpGrid->addWidget(cache, row, col++);
  col++;
  tmpGrid->addWidget(new QLabel("SeqExp: ", this), row, col++);
  m_seqExp = new QLabel(this, "seqexp");
  tmpGrid->addWidget(m_seqExp, row, col++);
  col++;
  tmpGrid->addWidget(new QLabel("SeqCur: ", this), row, col++);
  m_seqCur = new QLabel(this, "seqcur");
  tmpGrid->addWidget(m_seqCur, row, col++);
  row++; row++; col = 0;
  seqExpect(m_packet->serverSeqExp());
  m_seqCur->setText("????");

  // create labels to display client & server info
  tmpGrid->addWidget(new QLabel("Network ", this), row, col++);
  tmpGrid->addWidget(new QLabel("Client: ", this), row, col++);
  m_clientLabel = new QLabel(this);
  tmpGrid->addWidget(m_clientLabel, row, col++);
  col++;
  tmpGrid->addWidget(new QLabel("Track: ", this), row, col++);
  m_sessionLabel = new QLabel(this);
  tmpGrid->addWidget(m_sessionLabel, row, col++);
  col++;
  tmpGrid->addWidget(new QLabel("ServerPort: ", this), row, col++);
  m_serverLabel = new QLabel(this);
  tmpGrid->addWidget(m_serverLabel, row, col++);
  clientChanged(m_packet->clientAddr());
  serverPortLatched(m_packet->serverPort());

  // second row of network info
  row++; col = 1;
  tmpGrid->addWidget(new QLabel("Device: ", this), row, col++);
  QLabel* tmpLabel = new QLabel(this);
  tmpLabel->setText(showeq_params->device);
  tmpGrid->addWidget(tmpLabel, row, col++);
  col++;
  tmpGrid->addWidget(new QLabel("Realtime: ", this), row, col++);
  tmpLabel = new QLabel(this);
  tmpLabel->setText(QString::number(showeq_params->realtime));
  tmpGrid->addWidget(tmpLabel, row, col++);
  col++;
  tmpGrid->addWidget(new QLabel("MAC: ", this), row, col++);
  tmpLabel = new QLabel(this);
  tmpLabel->setText(showeq_params->mac_address);
  tmpGrid->addWidget(tmpLabel, row, col++);
  row++; row++; col = 0;

  // Decode
  tmpGrid->addWidget(new QLabel("Decode ", this), row, col++);
  tmpGrid->addWidget(new QLabel("Broken: ", this), row, col++);
  tmpLabel = new QLabel(this);
  tmpLabel->setText(QString::number(showeq_params->broken_decode));
  tmpGrid->addWidget(tmpLabel, row, col++);
  col++;
  tmpGrid->addWidget(new QLabel("HAVE_LIBEQ: ", this), row, col++);
  tmpLabel = new QLabel(this);
  tmpLabel->setText(QString::number(HAVE_LIBEQ));
  tmpGrid->addWidget(tmpLabel, row, col++);
  col++;
  tmpGrid->addWidget(new QLabel("Key: ", this), row, col++);
  m_decodeKeyLabel = new QLabel(this);
  m_decodeKeyLabel->setText(QString::number(m_packet->decodeKey(), 16));
  tmpGrid->addWidget(m_decodeKeyLabel, row, col++);
  row++; row++; col = 0;

  if (showeq_params->playbackpackets)
  {
    tmpGrid->addWidget(new QLabel("Playback ", this), row, col++);
    tmpGrid->addWidget(new QLabel("Rate: ", this), row, col++);
    m_playbackSpeed = new QSpinBox(-1, 9, 1, this, "speed");
    m_playbackSpeed->setSuffix("x");
    m_playbackSpeed->setSpecialValueText("Puase");
    m_playbackSpeed->setWrapping(true);
    tmpGrid->addWidget(m_playbackSpeed, row, col++, Qt::AlignLeft);

    int speed = m_packet->playbackSpeed();
    m_playbackSpeed->setValue(speed);

    QAccel* accel = new QAccel(this);
    int key;
    key = pSEQPrefs->getPrefKey("IncPlaybackSpeedKey", section, "Ctrl+X");
    accel->connectItem(accel->insertItem(key), m_packet, SLOT(incPlayback()));
    key = pSEQPrefs->getPrefKey("IncPlaybackSpeedKey", section, "Ctrl+Z");
    accel->connectItem(accel->insertItem(key), m_packet, SLOT(decPlayback()));
  }
  
  // supply the LCD's with signals
  connect (m_packet, SIGNAL(cacheSize(int)), 
	   cache, SLOT(setNum(int)));
  connect (m_packet, SIGNAL(seqExpect (int)), 
	   this, SLOT(seqExpect(int)));
  connect (m_packet, SIGNAL(seqReceive (int)), 
	   this, SLOT(seqReceive(int)));
  connect (m_packet, SIGNAL(clientChanged(uint32_t)),
	   this, SLOT(clientChanged(uint32_t)));
  connect (m_packet, SIGNAL(clientPortLatched(uint16_t)),
	   this, SLOT(clientPortLatched(uint16_t)));
  connect (m_packet, SIGNAL(serverPortLatched(uint16_t)),
	   this, SLOT(serverPortLatched(uint16_t)));
  connect (m_packet, SIGNAL(sessionTrackingChanged(uint8_t)),
	   this, SLOT(sessionTrackingChanged(uint8_t)));
  connect (m_packet, SIGNAL(numPacket(int)),
	   this, SLOT(numPacket(int)));
  connect (m_packet, SIGNAL(resetPacket(int)),
	   this, SLOT(resetPacket(int)));
  connect (m_packet, SIGNAL(keyChanged(void)),
	   this, SLOT(keyChanged(void)));

  if (m_playbackSpeed)
  {
    connect (m_playbackSpeed, SIGNAL(valueChanged(int)),
	     m_packet, SLOT(setPlayback(int)));
    connect (m_packet, SIGNAL(playbackSpeedChanged(int)),
	     m_playbackSpeed, SLOT(setValue(int)));
  }
}

NetDiag::~NetDiag()
{
}

void NetDiag::setCaption(const QString& text)
{
  // set the caption
  QFrame::setCaption(text);

  // set the preference
  pSEQPrefs->setPrefString("Caption", "NetDiag", caption());
}

void NetDiag::seqReceive(int seq)
{
  QString disp;
  disp.sprintf("%4.4x", seq);
  m_seqCur->setText(disp);
}

void NetDiag::seqExpect(int seq)
{
  QString disp;
  disp.sprintf("%4.4x", seq);
  m_seqExp->setText(disp);
}

void NetDiag::clientChanged(uint32_t addr)
{
  QString disp, tmp;
  disp = print_addr(addr);

  uint8_t sessionState = m_packet->session_tracking_enabled();
  m_sessionLabel->setNum(sessionState);
  switch (sessionState)
  {
  case 2:
    tmp.sprintf(":%d", 
		m_packet->clientPort());
    disp += tmp;
    break;
  case 1:
    disp += QString(":?");
    break;
  default:
    break;
  }

  m_clientLabel->setText(disp);
}

void NetDiag::clientPortLatched(uint16_t clientPort)
{
  QString disp, tmp;
  uint32_t addr = m_packet->clientAddr();

  disp = print_addr(addr);
  tmp.sprintf(":%d", clientPort);

  disp += tmp;

  m_clientLabel->setText(disp);
}

void NetDiag::serverPortLatched(uint16_t port)
{
  m_serverLabel->setText(QString::number(port));
}

void NetDiag::sessionTrackingChanged(uint8_t sessionTrackState)
{
  QString disp, tmp;
  disp = print_addr(m_packet->clientAddr());

  m_sessionLabel->setNum(sessionTrackState);
  switch(sessionTrackState)
  {
  case 1:
    m_clientLabel->setText(disp + ":?");
    break;
  case 2:
    break;
  default:
    m_clientLabel->setText(disp);
    break;
  }
}

void NetDiag::resetPacket(int num)
{
  // if passed 0 reset the average
  m_packetStartTime = mTime();
  m_initialcount = num;

  m_packetTotal->setText(QString::number(num));
  m_packetRecent->setText("0");
  m_packetAvg->setText("0.0");
}

void NetDiag::numPacket(int num)
{
  // start the timer of not started
  if (!m_packetStartTime)
    m_packetStartTime = mTime();

   QString tempStr;

   m_packetTotal->setText(QString::number(num));

  // update five times per sec
  static int lastupdate = 0;
  if ( (mTime() - lastupdate) < 1000)
    return;
  lastupdate = mTime();

   int numdelta = num - m_initialcount;
   m_packetRecent->setText(QString::number(num));
   int delta = mTime() - m_packetStartTime;
   if (numdelta && delta)
     tempStr.sprintf("%2.1f", 
		     (float) (numdelta<<10) / (float) delta);
   else   
     tempStr.sprintf("0.0");

   m_packetAvg->setText(tempStr);
}

void NetDiag::keyChanged(void)
{
  m_decodeKeyLabel->setText(QString::number(m_packet->decodeKey(), 16));
}

QString NetDiag::print_addr(in_addr_t  addr)
{
#ifdef DEBUG_PACKET
   debug ("print_addr()");
#endif /* DEBUG_PACKET */
  QString paddr;

  paddr.sprintf ( "%d.%d.%d.%d",
		  addr & 0x000000ff,
		  (addr & 0x0000ff00) >> 8,
		  (addr & 0x00ff0000) >> 16,
		  (addr & 0xff000000) >> 24
                  );

   return paddr;
}
