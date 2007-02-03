/*
 * skilllist.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#ifndef EQSKILLLIST_H
#define EQSKILLLIST_H

#include <qwidget.h>
#include <q3listview.h>

#include "seqlistview.h"
#include "seqwindow.h"
#include "player.h"

#define SKILLCOL_NAME 0
#define SKILLCOL_VALUE 1

class SkillList : public SEQListView
{
   Q_OBJECT

 public:
   // constructor
   SkillList (Player* player,
		QWidget*  parent = 0, 
		const char* name = 0); 
   
   // destructor
   ~SkillList();

   bool showLanguages() { return m_showLanguages; }

 public slots:
   void addSkill(int skillId, int value);
   void changeSkill(int skillId, int value);
   void deleteSkills(void);
   void addLanguage(int langId, int value);
   void changeLanguage(int langId, int value);
   void deleteLanguages(void);
   void addLanguages(void);
   void showLanguages(bool show);

 private:
   // the player this skill list is monitoring
   Player* m_pPlayer;

   // the list view items related to skills
   Q3ListViewItem* m_skillList[MAX_KNOWN_SKILLS];

   // the list view items related to languages
   Q3ListViewItem* m_languageList[MAX_KNOWN_LANGS];

   // whether or not to show languages
   bool m_showLanguages;
};

class SkillListWindow : public SEQWindow
{
  Q_OBJECT

 public:
  SkillListWindow(Player* player, QWidget* parent = 0, const char* name = 0);
  ~SkillListWindow();
  SkillList* skillList() { return m_skillList; }

 public slots:
  virtual void savePrefs(void);

 protected:
  SkillList* m_skillList;
};

#endif // EQSKILLLIST_H
