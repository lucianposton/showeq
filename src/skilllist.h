/*
 * skilllist.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#ifndef EQSKILLLIST_H
#define EQSKILLLIST_H

#include <qwidget.h>
#include <qlistview.h>

#include "player.h"

class EQSkillList : public QListView
{
   Q_OBJECT

 public:
   // constructor
   EQSkillList (EQPlayer* player,
		QWidget*  parent = 0, 
		const char* name = 0); 
   
   // destructor
   ~EQSkillList();

   bool showLanguages() { return m_showLanguages; }

 public slots:
   void addSkill(int skillId, int value);
   void changeSkill(int skillId, int value);
   void deleteSkills(void);
   void addLanguage(int langId, int value);
   void changeLanguage(int langId, int value);
   void deleteLanguages(void);
   void showLanguages(bool show);
   void savePrefs(void);

 private:
   // the player this skill list is monitoring
   EQPlayer* m_pPlayer;

   // the list view items related to skills
   QListViewItem* m_skillList[MAX_KNOWN_SKILLS];

   // the list view items related to languages
   QListViewItem* m_languageList[MAX_KNOWN_LANGS];

   // whether or not to show languages
   bool m_showLanguages;
};

#endif // EQSKILLLIST_H
