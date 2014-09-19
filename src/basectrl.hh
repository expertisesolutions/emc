#ifndef _BASECTRL_HH
#define _BASECTRL_HH

#include <iostream>

#include <elm_layout.eo.hh>
#include <elm_win.eo.hh>

namespace emc {

class basectrl
{
   std::string groupname;
   std::string theme;
   std::function<void()> deactive_cb;

   protected:
     ::elm_layout layout;

   public:
     basectrl(const ::elm_layout &layout, const std::string &theme, const std::string &groupname);
     ~basectrl() {};
     void active();
     void deactive();
     bool theme_set(const std::string &theme);
     bool deactive_cb_set(std::function<void()> deactive_cb);
};

} //emc
#endif
