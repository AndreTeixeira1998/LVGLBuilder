#include "LVGLTabview.h"

#include <QIcon>

#include "LVGLCore.h"
#include "LVGLObject.h"
#include "properties/LVGLPropertyAnyFunc.h"
#include "properties/LVGLPropertyTextList.h"

class LVGLPropertyTabBtnPos : public LVGLPropertyEnum {
 public:
  LVGLPropertyTabBtnPos()
      : LVGLPropertyEnum({"None", "Top", "Bottom", "Left", "Right"}),
        m_values({"LV_TABVIEW_TAB_POS_NONE", "LV_TABVIEW_TAB_POS_TOP",
                  "LV_TABVIEW_TAB_POS_BOTTOM", "LV_TABVIEW_TAB_POS_LEFT",
                  "LV_TABVIEW_TAB_POS_RIGHT"}) {}

  QString name() const { return "Tab position"; }

  QStringList function(LVGLObject *obj) const {
    if (get(obj) != LV_TABVIEW_TAB_POS_TOP)
      return QStringList() << QString("lv_tabview_set_btns_pos(%1, %2);")
                                  .arg(obj->codeName())
                                  .arg(m_values.at(get(obj)));
    return QStringList();
  }

 protected:
  int get(LVGLObject *obj) const { return lv_tabview_get_btns_pos(obj->obj()); }
  void set(LVGLObject *obj, int index) {
    lv_tabview_set_btns_pos(obj->obj(), index & 0xff);
  }

  QStringList m_values;
};

class LVGLPropertyTabs : public LVGLPropertyAnyFunc {
 public:
  LVGLPropertyTabs(const AnyFuncColType arr[], int size)
      : LVGLPropertyAnyFunc(arr, size) {}
  QString name() const { return "Add Tabs"; }

 protected:
  QStringList get(LVGLObject *obj) const {
    if (!m_list.isEmpty() && m_list[0] != "Empty list") return m_list;
    return QStringList();
  }
  void set(LVGLObject *obj, QStringList list) {
    m_list = list;
    for (int i = 0; i < m_list.size(); ++i) {
      QStringList strlist = m_list[i].split('@');
      int index = strlist[0].toInt();
      m_tabNames[index] = strlist[1];
    }

    if (!m_tabNames.isEmpty()) {
      if (m_result.size() > m_tabNames.size()) {
      } else {
        for (int i = 1; i <= m_tabNames.size(); ++i) {
          auto byte = m_tabNames[i].toUtf8();
          char *name = new char[byte.size() + 1];
          name[byte.size()] = '\0';
          strcpy(name, byte.data());
          if (m_result.contains(i)) {
            lv_tabview_set_tab_name(obj->obj(), i - 1, name);
          } else {
            m_result.insert(i);
            lv_tabview_add_tab(obj->obj(), name);
          }
        }
      }
    }
  }

 private:
  QStringList m_list;
  QMap<int, QString> m_tabNames;
  QSet<int> m_result;
  ;
};

class LVGLPropertyTabCurrent : public LVGLPropertyInt {
 public:
  LVGLPropertyTabCurrent() : LVGLPropertyInt(0, UINT16_MAX) {}

  QString name() const { return "Current tab"; }

  QStringList function(LVGLObject *obj) const {
    return QStringList() << QString(
                                "lv_tabview_set_tab_act(%1, %2, LV_ANIM_OFF);")
                                .arg(obj->codeName())
                                .arg(get(obj));
  }

 protected:
  int get(LVGLObject *obj) const { return lv_tabview_get_tab_act(obj->obj()); }
  void set(LVGLObject *obj, int value) {
    lv_tabview_set_tab_act(obj->obj(), static_cast<uint16_t>(value),
                           LV_ANIM_OFF);
  }
};

class LVGLPropertyTabAnimTime : public LVGLPropertyInt {
 public:
  LVGLPropertyTabAnimTime() : LVGLPropertyInt(0, UINT16_MAX) {}

  QString name() const { return "Animation time"; }

  QStringList function(LVGLObject *obj) const {
    return QStringList() << QString("lv_tabview_set_anim_time(%1, %2);")
                                .arg(obj->codeName())
                                .arg(get(obj));
  }

 protected:
  int get(LVGLObject *obj) const {
    return lv_tabview_get_anim_time(obj->obj());
  }
  void set(LVGLObject *obj, int value) {
    lv_tabview_set_anim_time(obj->obj(), static_cast<uint16_t>(value));
  }
};

class LVGLPropertyTabScrollbars : public LVGLPropertyEnum {
 public:
  LVGLPropertyTabScrollbars()
      : LVGLPropertyEnum(QStringList() << "Off"
                                       << "On"
                                       << "Drag"
                                       << "Auto"),
        m_values({"LV_SB_MODE_OFF", "LV_SB_MODE_ON", "LV_SB_MODE_DRAG",
                  "LV_SB_MODE_AUTO"}) {}

  QString name() const { return "Scrollbars"; }

 protected:
  int get(LVGLObject *obj) const {
    lv_scrollbar_mode_t mode = LV_SCROLLBAR_MODE_AUTO;
    for (uint16_t i = 0; i < lv_tabview_get_tab_count(obj->obj()); ++i) {
      lv_obj_t *page = lv_tabview_get_tab(obj->obj(), i);
      mode = lv_page_get_scrollbar_mode(page);
    }
    return mode;
  }
  void set(LVGLObject *obj, int index) {
    for (uint16_t i = 0; i < lv_tabview_get_tab_count(obj->obj()); ++i) {
      lv_obj_t *page = lv_tabview_get_tab(obj->obj(), i);
      lv_page_set_scrollbar_mode(page, index & 0xff);
    }
  }

  QStringList m_values;
};

LVGLTabview::LVGLTabview() {
  initStateStyles();
  m_parts << LV_TABVIEW_PART_BG << LV_TABVIEW_PART_BG_SCROLLABLE
          << LV_TABVIEW_PART_TAB_BG << LV_TABVIEW_PART_TAB_BTN
          << LV_TABVIEW_PART_INDIC;
  static AnyFuncColType arr[] = {e_Seqlabel, e_QLineEdit};
  m_properties << new LVGLPropertyTabs(arr, 2);
  m_properties << new LVGLPropertyTabBtnPos;
  m_properties << new LVGLPropertyTabCurrent;
  m_properties << new LVGLPropertyTabScrollbars;

  m_editableStyles << LVGL::BtnMatrixBTN;  // LV_TABVIEW_PART_BG
  m_editableStyles << LVGL::Background;    // LV_TABVIEW_PART_BG_SCROLLABLE
  m_editableStyles << LVGL::Background;    // LV_TABVIEW_PART_TAB_BG
  m_editableStyles << LVGL::Background;    // LV_TABVIEW_PART_TAB_BTN
  m_editableStyles << LVGL::Background;    // LV_TABVIEW_PART_INDIC
}

QString LVGLTabview::name() const { return "Tabview"; }

QString LVGLTabview::className() const { return "lv_tabview"; }

LVGLWidget::Type LVGLTabview::type() const { return TabView; }

QIcon LVGLTabview::icon() const { return QIcon(); }

lv_obj_t *LVGLTabview::newObject(lv_obj_t *parent) const {
  lv_obj_t *obj = lv_tabview_create(parent, nullptr);
  return obj;
}

QSize LVGLTabview::minimumSize() const { return QSize(100, 150); }

QStringList LVGLTabview::styles() const {
  return QStringList() << "LV_TABVIEW_PART_BG"
                       << "LV_TABVIEW_PART_BG_SCROLLABLE"
                       << "LV_TABVIEW_PART_TAB_BG"
                       << "LV_TABVIEW_PART_TAB_BTN"
                       << "LV_TABVIEW_PART_INDIC";
}

lv_style_t *LVGLTabview::style(lv_obj_t *obj, lv_obj_part_t part) const {
  return lv_obj_get_local_style(obj, part);
}

void LVGLTabview::setStyle(lv_obj_t *obj, int type, lv_style_t *style) const {
  lv_obj_add_style(obj, LV_BTN_PART_MAIN, style);
}

void LVGLTabview::addStyle(lv_obj_t *obj, lv_style_t *style,
                           lv_obj_part_t part) const {
  lv_obj_add_style(obj, part, style);
}

void LVGLTabview::initStateStyles() {
  for (int i = 0; i < 5; ++i) {
    lv_style_t *de = new lv_style_t;
    lv_style_t *ch = new lv_style_t;
    lv_style_t *fo = new lv_style_t;
    lv_style_t *ed = new lv_style_t;
    lv_style_t *ho = new lv_style_t;
    lv_style_t *pr = new lv_style_t;
    lv_style_t *di = new lv_style_t;
    lv_style_init(de);
    lv_style_init(ch);
    lv_style_init(fo);
    lv_style_init(ed);
    lv_style_init(ho);
    lv_style_init(pr);
    lv_style_init(di);
    QList<lv_style_t *> stateStyles;
    stateStyles << de << ch << fo << ed << ho << pr << di;
    m_partsStyles[i] = stateStyles;
  }
}
