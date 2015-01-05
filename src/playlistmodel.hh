#ifndef _PLAYLISTMODEL_HH
#define _PLAYLISTMODEL_HH

#include <iostream>
#include <iterator>

#include <Eina.h>
#include <Emodel.h>
#include <Esql_Model.h>

#include <Emodel.hh>
#include <Esql_Model.hh>
#include <eina_accessor.hh>

#include <eo_inherit.hh>
#include <elm_layout.eo.hh>
#include "settingsmodel.hh"

namespace emc {

class playlistmodel
        //: efl::eo::inherit<playlistmodel, ::eo::base>, public ::emodel
        : public efl::eo::inherit<playlistmodel, ::esql::model_database>
{
    efl::eina::ptr_list<esql::model_row, efl::eina::heap_copy_allocator> tracks;
    efl::eina::ptr_list<esql::model_row>::iterator current_track;
    settingsmodel &settings;
    ::elm_video &player;

    public:
        playlistmodel(settingsmodel &settings);
        ~playlistmodel() {};
        void load() { std::cout << "LOAD" << std::endl; };
        void unload() {};
        void properties_load() {};
        void children_load() {};

        Emodel_Load_Status load_status_get() { printf("FUUUUUUUUUUUUUUUUU\n");return EMODEL_LOAD_STATUS_LOADED; };
        Emodel_Load_Status properties_list_get(Eina_List * const* properties_list) { return EMODEL_LOAD_STATUS_LOADED; };
        Emodel_Load_Status property_get(std::string property, Eina_Value* value) {  return EMODEL_LOAD_STATUS_LOADED; };
        Emodel_Load_Status property_set(std::string property, Eina_Value value) {  return EMODEL_LOAD_STATUS_LOADED; };

        efl::eo::concrete child_add() { return efl::eo::concrete(nullptr); };
        Emodel_Load_Status child_del(eo::base child) {  return EMODEL_LOAD_STATUS_LOADED; };
        Emodel_Load_Status children_slice_get(unsigned start, unsigned count, Eina_Accessor ** children_accessor);
        Emodel_Load_Status children_count_get(unsigned* children_count);

        void list_set(std::vector<esql::model_row> tracks);
        void track_add(esql::model_row track);
        void play(esql::model_row track);
        void pause();
        esql::model_row play();
        esql::model_row play(efl::eina::ptr_list<esql::model_row>::iterator track);
        esql::model_row play_next();
        esql::model_row play_prev();
        esql::model_row curr();

        void opened_done_cb();
};

}

#endif
