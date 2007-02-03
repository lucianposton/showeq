/*
 * spelllist.h
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

/*
 * Orig Author - Crazy Joe Divola (cjd1@users.sourceforge.net)
 * Date - 9/7/2001
 */

#ifndef SPELLLIST_H
#define SPELLLIST_H

#include <q3valuelist.h>
#include <q3listview.h>
#include <q3popupmenu.h>
#include <time.h>
#include <sys/time.h>

#include "seqwindow.h"
#include "seqlistview.h"
#include "spellshell.h"
#include "everquest.h"

#define SPELLCOL_SPELLNAME  0
#define SPELLCOL_SPELLID    1
#define SPELLCOL_CASTERID   2
#define SPELLCOL_CASTERNAME 3
#define SPELLCOL_TARGETID   4
#define SPELLCOL_TARGETNAME 5
#define SPELLCOL_CASTTIME   6
#define SPELLCOL_DURATION   7

class SpellListItem : public Q3ListViewItem 
{
 public:
  SpellListItem(Q3ListViewItem *parent);
  SpellListItem(Q3ListView *parent = NULL);
  virtual void paintCell( QPainter *p, const QColorGroup &cg,
			  int column, int width, int alignment );
  const QColor textColor();
  void setTextColor(const QColor &color);
  void update();
  void setSpellItem(const SpellItem *);
  const SpellItem* item() const;
  const QString& category() const;
  void setCategory(QString& cat);
 private:
  QColor m_textColor;
  bool m_btextSet;
  const SpellItem *m_item;
  QString m_category;
};

class SpellList : public SEQListView 
{
  Q_OBJECT
 public:
  SpellList(SpellShell* sshell, QWidget *parent = 0, const char *name = 0);
  Q3PopupMenu* menu();
  void SelectItem(const SpellItem *item);
  SpellListItem* Selected();
  SpellListItem* InsertSpell(const SpellItem *item);
  void DeleteItem(const SpellItem *item);
  //SpellItem* AddCategory(QString& name, QColor color = Qt::black);
  //void RemCategory(SpellListItem *);
  //void clearCategories();
  QColor pickSpellColor(const SpellItem *item, QColor def = Qt::black) const;
  //QString& getCategory(SpellListItem *);
  SpellListItem* Find(const SpellItem *);
  
 signals:
  void listUpdated();   // flags in spawns have changed
  void listChanged();   // categories have changed
  
 public slots:
  // SpellShell signals
  void addSpell(const SpellItem *);
  void delSpell(const SpellItem *);
  void changeSpell(const SpellItem *);
  void selectSpell(const SpellItem *);
  void clear();

  void mouseDoubleClicked(Q3ListViewItem *item);
  void rightButtonClicked(Q3ListViewItem *, const QPoint&, int);
  void activated(int);

 protected slots:  
   void init_menu(void);

 private:
  void selectAndOpen(SpellListItem *);
  SpellShell* m_spellShell;
  Q3ValueList<QString> m_categoryList;
  Q3ValueList<SpellListItem *> m_spellList;
  Q3PopupMenu *m_menu;
  
  int mid_spellName, mid_spellId;
  int mid_casterId, mid_casterName;
  int mid_targetId, mid_targetName;
  int mid_casttime, mid_duration;
};

class SpellListWindow : public SEQWindow
{
  Q_OBJECT

 public:
  SpellListWindow(SpellShell* sshell, QWidget* parent = 0, const char* name = 0);
  ~SpellListWindow();
  virtual Q3PopupMenu* menu();

  SpellList* spellList() { return m_spellList; }

 public slots:
  virtual void savePrefs(void);

 protected:
  SpellList* m_spellList;
};

#endif
