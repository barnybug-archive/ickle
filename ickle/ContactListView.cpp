/* $Id: ContactListView.cpp,v 1.54 2003-01-12 17:42:17 barnabygray Exp $
 * 
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "ContactListView.h"

#include <gtkmm/stock.h>

#include <libicq2000/Client.h>

#include "ickle.h"

#include "Settings.h"
#include "Icons.h"
#include "main.h"

#include "AddContactDialog.h"
#include "RemoveContactDialog.h"
#include "AddGroupDialog.h"
#include "RemoveGroupDialog.h"
#include "RenameGroupDialog.h"
#include "SendAuthReqDialog.h"

#include "pixmaps/info.xpm"

ContactListView::ContactListView(Gtk::Window& parent, MessageQueue& mq)
  : m_parent(parent),
    m_message_queue(mq),
    m_offline_contacts(false),
    m_single_click(false),
    m_check_away_click(true)
{
  // setup callbacks

  // -- library callbacks      --
  icqclient.contactlist.connect(this,&ContactListView::contactlist_cb);
  icqclient.contact_status_change_signal.connect(this,&ContactListView::contact_status_change_cb);
  icqclient.contact_userinfo_change_signal.connect(this,&ContactListView::contact_userinfo_change_cb);
  icqclient.want_auto_resp.connect( this, &ContactListView::want_auto_resp_cb );

  // -- MessageQueue callbacks --
  m_message_queue.added.connect(SigC::slot(*this, &ContactListView::queue_added_cb));
  m_message_queue.removed.connect(SigC::slot(*this, &ContactListView::queue_removed_cb));

  // -- gui callbacks          --
  g_icons.icons_changed.connect(SigC::slot(*this,&ContactListView::icons_changed_cb));
  g_settings.settings_changed.connect(SigC::slot(*this,&ContactListView::settings_changed_cb));


  // setup Tree store
  m_reftreestore = Gtk::TreeStore::create(m_columns);
  m_reftreestore->set_sort_func( m_columns.icon.index(), SigC::slot(*this, &ContactListView::sort_func));
  m_reftreestore->set_sort_func( m_columns.nick.index(), SigC::slot(*this, &ContactListView::sort_func));
  m_reftreestore->set_sort_column_id( m_columns.nick.index(), Gtk::SORT_ASCENDING);
  
  // setup Tree view
  set_model( m_reftreestore );
  
  // create columns
  {
    Gtk::TreeViewColumn *pColumn;
    
    Gtk::CellRendererPixbuf* pPixbufRenderer = Gtk::manage( new Gtk::CellRendererPixbuf() );
    Gtk::CellRendererText* pTextRenderer = Gtk::manage( new Gtk::CellRendererText() );
    pColumn = Gtk::manage( new Gtk::TreeView::Column( _("Nick") ) );

    pColumn->pack_start( *pPixbufRenderer, false );
    pColumn->pack_start( *pTextRenderer );
    
    pColumn->add_attribute( pPixbufRenderer->property_pixbuf(), m_columns.icon );
    pColumn->add_attribute( pPixbufRenderer->property_visible(), m_columns.is_contact );

    pColumn->add_attribute( pTextRenderer->property_text(), m_columns.nick );
    pColumn->add_attribute( pTextRenderer->property_weight(), m_columns.text_weight );
    pColumn->add_attribute( pTextRenderer->property_foreground_gdk(), m_columns.text_colour );

    append_column( *pColumn );
  }

  {
    // create right-click menus

    using namespace Gtk::Menu_Helpers;

    // popup for right-click contact
    MenuList& ml_c = m_rc_popup_contact.items();
    ml_c.push_back( ImageMenuElem( _("Check away message"),
				   * manage( new Gtk::Image( g_icons.get_icon_for_status( ICQ2000::STATUS_AWAY, false ) ) ),  /* just a placeholder */
				   SigC::slot( *this, &ContactListView::contact_fetch_away_msg_cb ) ) );
    m_rc_popup_away = dynamic_cast<Gtk::ImageMenuItem*>(&(ml_c.back()));
    ml_c.push_back( ImageMenuElem( _("User Info"),
				   * manage( new Gtk::Image( Gdk::Pixbuf::create_from_xpm_data( info_xpm ) ) ),
				   SigC::slot( *this, &ContactListView::contact_userinfo_cb ) ) );
    ml_c.push_back( MenuElem( _("Send Auth Request"),  SigC::slot( *this, &ContactListView::contact_send_auth_req_cb ) ) );
    m_rc_popup_auth = &(ml_c.back());
    ml_c.push_back( ImageMenuElem( _("Remove Contact"),
				   * manage( new Gtk::Image(Gtk::Stock::REMOVE, Gtk::ICON_SIZE_MENU) ),
				   SigC::slot( *this, &ContactListView::contact_remove_cb ) ) );
    ml_c.push_back( SeparatorElem() );
    ml_c.push_back( ImageMenuElem( _("Add Contact"),
				   * manage( new Gtk::Image(Gtk::Stock::ADD, Gtk::ICON_SIZE_MENU) ),
				   SigC::slot( *this, &ContactListView::contact_add_cb ) ) );
    ml_c.push_back( ImageMenuElem( _("Add Group"),
				   * manage( new Gtk::Image(Gtk::Stock::ADD, Gtk::ICON_SIZE_MENU) ),
				   SigC::slot( *this, &ContactListView::group_add_cb ) ) );

    // popup for right-click group
    MenuList& ml_g = m_rc_popup_group.items();
    ml_g.push_back( MenuElem( _("Rename Group"),       SigC::slot( *this, &ContactListView::group_rename_cb ) ) );
    ml_g.push_back( ImageMenuElem( _("Remove Group"),
			      * manage( new Gtk::Image(Gtk::Stock::REMOVE, Gtk::ICON_SIZE_MENU) ),
			      SigC::slot( *this, &ContactListView::group_remove_cb ) ) );
    ml_g.push_back( SeparatorElem() );
    ml_g.push_back( ImageMenuElem( _("Add Contact"),
			      * manage( new Gtk::Image(Gtk::Stock::ADD, Gtk::ICON_SIZE_MENU) ),
			      SigC::slot( *this, &ContactListView::contact_add_cb ) ) );
    ml_g.push_back( ImageMenuElem( _("Add Group"),
			      * manage( new Gtk::Image(Gtk::Stock::ADD, Gtk::ICON_SIZE_MENU) ),
			      SigC::slot( *this, &ContactListView::group_add_cb ) ) );

    // popup for right-click over blank area
    MenuList& ml_b = m_rc_popup_blank.items();
    ml_b.push_back( ImageMenuElem( _("Add Contact"),
			      * manage( new Gtk::Image(Gtk::Stock::ADD, Gtk::ICON_SIZE_MENU) ),
			      SigC::slot( *this, &ContactListView::contact_add_cb ) ) );
    ml_b.push_back( ImageMenuElem( _("Add Group"),
			      * manage( new Gtk::Image(Gtk::Stock::ADD, Gtk::ICON_SIZE_MENU) ),
			      SigC::slot( *this, &ContactListView::group_add_cb ) ) );
    // anything else?
  }
  

}

ContactListView::~ContactListView()
{
}


void ContactListView::contactlist_cb(ICQ2000::ContactListEvent *ev)
{
  if (ev->getType() == ICQ2000::ContactListEvent::UserAdded)
  {
    ICQ2000::UserAddedEvent *cev = static_cast<ICQ2000::UserAddedEvent*>(ev);
    if (m_offline_contacts || cev->getContact()->getStatus() != ICQ2000::STATUS_OFFLINE)
    {
      /* add contact */
      add_contact( cev->getContact(), cev->get_group() );
    }
  }
  else if (ev->getType() == ICQ2000::ContactListEvent::UserRemoved)
  {
    /* remove contact */
    ICQ2000::UserRemovedEvent *cev = static_cast<ICQ2000::UserRemovedEvent*>(ev);
    remove_contact(cev->getContact());
  }
  else if (ev->getType() == ICQ2000::ContactListEvent::GroupAdded)
  {
    /* add group */
    ICQ2000::GroupAddedEvent *cev = static_cast<ICQ2000::GroupAddedEvent*>(ev);
    add_group(cev->get_group());
  }
  else if (ev->getType() == ICQ2000::ContactListEvent::GroupRemoved)
  {
    /* remove group */
    ICQ2000::GroupRemovedEvent *cev = static_cast<ICQ2000::GroupRemovedEvent*>(ev);
    remove_group(cev->get_group());
  }
  else if (ev->getType() == ICQ2000::ContactListEvent::GroupChange)
  {
    /* update group */
    ICQ2000::GroupChangeEvent *cev = static_cast<ICQ2000::GroupChangeEvent*>(ev);
    update_group(cev->get_group());
  }
  
}

bool ContactListView::on_button_press_event(GdkEventButton * ev)
{
  Gtk::TreeModel::Path path;
  Gtk::TreeView::Column * col;
  int cell_x, cell_y;
  
  if (get_path_at_pos((int)ev->x, (int)ev->y, path, col, cell_x, cell_y))
  {
    Gtk::TreeModel::iterator iter = m_reftreestore->get_iter(path);
    Gtk::TreeModel::Row row = *iter;
    if (row[m_columns.is_contact])
    {
      /* clicked on a contact */
      if (ev->button == 1)
      {
	/* left click */

	/* check away-message stuff */

	if (m_single_click)
	{
	}
      }
      else if (ev->button == 3)
      {
	/* right click - popup contact menu */
	popup_contact_menu( ev->button, ev->time, row[m_columns.id] );
      }
    }
    else
    {
      /* clicked on a group */
      if (ev->button == 3)
      {
	/* right click - popup group menu */
	m_rc_popup_group.popup( ev->button, ev->time );
      }
      
    }
    
  }
  else
  {
    /* click on blank */
    if (ev->button == 3)
    {
      /* right click - popup blank menu */
      m_rc_popup_blank.popup( ev->button, ev->time );
    }
    
  }
  
  
  return TreeView::on_button_press_event(ev);
}

void ContactListView::popup_contact_menu(guint button, guint32 activate_time, unsigned int uin)
{
  ICQ2000::ContactRef c = icqclient.getContactTree().lookup_uin(uin);

  /* set check away message icon */
  if ( c->getStatus() != ICQ2000::STATUS_OFFLINE && c->getStatus() != ICQ2000::STATUS_ONLINE
       && icqclient.getSelfContact()->getStatus() != ICQ2000::STATUS_OFFLINE )
  {
    Gtk::Image * img = Gtk::manage( new Gtk::Image( g_icons.get_icon_for_status( c->getStatus(), false ) ) );
    img->show();
    m_rc_popup_away->set_image( * img );
    m_rc_popup_away->set_sensitive(true);
  }
  else
  {
    m_rc_popup_away->set_image( * Gtk::manage( new Gtk::Image() ) ); /* a blank */
    m_rc_popup_away->set_sensitive(false);
  }

  m_rc_popup_contact.popup( button, activate_time );
}

void ContactListView::on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn * col)
{
  Gtk::TreeModel::iterator iter = m_reftreestore->get_iter(path);
  Gtk::TreeModel::Row row = *iter;

  if (row[m_columns.is_contact])
  {
    m_signal_messagebox_popup.emit(row[m_columns.id]);
  }
}

void ContactListView::add_group(const ICQ2000::ContactTree::Group& gp)
{
  Gtk::TreeModel::iterator iter = m_reftreestore->append();
  Gtk::TreeModel::Row row = *iter;
  row[m_columns.nick] = gp.get_label();
  row[m_columns.nick_sort_key] = Glib::ustring(gp.get_label()).casefold_collate_key();
  row[m_columns.is_contact] = false;
  row[m_columns.id] = gp.get_id();
  m_group_map[ gp.get_id() ] = iter;
}

void ContactListView::add_contact(const ICQ2000::ContactRef& c, const ICQ2000::ContactTree::Group& gp)
{
  if (m_group_map.count( gp.get_id() ))
  {
    Gtk::TreeModel::iterator parent = m_group_map[ gp.get_id() ];
    Gtk::TreeModel::iterator iter = m_reftreestore->append(parent->children());
    Gtk::TreeModel::Row row = *iter;

    m_contact_map[ c->getUIN() ] = iter;
    row[m_columns.is_contact] = true;
    row[m_columns.id] = c->getUIN();
    row[m_columns.status] = ICQ2000::STATUS_OFFLINE;
    row[m_columns.text_colour] = get_style()->get_text( Gtk::STATE_NORMAL );
    update_contact(c);
  }
}

void ContactListView::update_contact(const ICQ2000::ContactRef& c)
{
  if (m_contact_map.count( c->getUIN() ))
  {
    Gtk::TreeModel::iterator iter = m_contact_map[ c->getUIN() ];
    Gtk::TreeModel::Row row = *iter;
    update_icon(row, c);
    row[m_columns.nick] = c->getNameAlias();
    row[m_columns.nick_sort_key] = Glib::ustring(c->getNameAlias()).casefold_collate_key();

    if (row[m_columns.status] == ICQ2000::STATUS_OFFLINE
	&& c->getStatus() != ICQ2000::STATUS_OFFLINE)
    {
      /* highlight contacts who've just appeared online */
      row[m_columns.text_weight] = Pango::WEIGHT_BOLD;
      /* set timer */
      Glib::signal_timeout().connect( SigC::bind( SigC::slot( *this, &ContactListView::contact_restore_weight_timeout_cb ), c ), 5000 );
    }
    else
    {
      /* default weight */
      row[m_columns.text_weight] = Pango::WEIGHT_NORMAL;
    }

    row[m_columns.status] = c->getStatus();
    row[m_columns.messages] = m_message_queue.get_contact_size(c);
  }
}

void ContactListView::update_icon(Gtk::TreeModel::Row& row, const ICQ2000::ContactRef& c)
{
  unsigned int msgs = m_message_queue.get_contact_size(c);
  Glib::RefPtr<Gdk::Pixbuf> p;

  if (msgs > 0)
  {
    MessageEvent *ev = m_message_queue.get_contact_first_message(c);
    if (ev->getServiceType() == MessageEvent::ICQ)
    {
      ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
      p = g_icons.get_icon_for_event(icq->getICQMessageType());
    }
  }
  else
  {
    p = g_icons.get_icon_for_status( c->getStatus(), c->isInvisible() );
  }

  row[m_columns.icon] = p;
}

void ContactListView::remove_group(const ICQ2000::ContactTree::Group& gp)
{
  if (m_group_map.count( gp.get_id() ))
  {
    Gtk::TreeModel::iterator iter = m_group_map[ gp.get_id() ];
    m_reftreestore->erase(iter);
    m_group_map.erase( gp.get_id() );
  }
}

void ContactListView::update_group(const ICQ2000::ContactTree::Group& gp)
{
  if (m_group_map.count( gp.get_id() ))
  {
    Gtk::TreeModel::iterator iter = m_group_map[ gp.get_id() ];
    Gtk::TreeModel::Row row = *iter;
    row[m_columns.nick] = gp.get_label();
    row[m_columns.nick_sort_key] = Glib::ustring(gp.get_label()).casefold_collate_key();
  }
}

void ContactListView::update_list()
{
  /* could be done more efficiently! */
  m_group_map.clear();
  m_contact_map.clear();
  
  m_reftreestore->clear();

  ICQ2000::ContactTree& ct = icqclient.getContactTree();
  ICQ2000::ContactTree::const_iterator curr = ct.begin();
  while (curr != ct.end())
  {
    add_group( *curr );
    
    ICQ2000::ContactTree::Group::const_iterator gcurr = curr->begin();
    while (gcurr != curr->end())
    {
      if ( m_offline_contacts || (*gcurr)->getStatus() != ICQ2000::STATUS_OFFLINE
	   || m_message_queue.get_contact_size( *gcurr ) > 0)
      {
	/* add contact */
	add_contact( *gcurr, *curr );
      }
      
      ++gcurr;
    }
    
    ++curr;
  }
}

void ContactListView::remove_contact(const ICQ2000::ContactRef& c)
{
  if (m_contact_map.count( c->getUIN() ))
  {
    Gtk::TreeModel::iterator iter = m_contact_map[ c->getUIN() ];
    m_reftreestore->erase(iter);
    m_contact_map.erase( c->getUIN() );
  }
}

void ContactListView::contact_userinfo_change_cb(ICQ2000::UserInfoChangeEvent *ev)
{
}

void ContactListView::want_auto_resp_cb(ICQ2000::ICQMessageEvent *ev)
{
  /* blink contact on contact list - nice little touch to see who is
     checking your away message - borrowed from licq :-) */
  ICQ2000::ContactRef c = ev->getContact();
  if ( m_contact_map.count(c->getUIN()) )
  {
    Gtk::TreeModel::Row row = * m_contact_map[ c->getUIN() ];
    row[m_columns.text_colour] = Gdk::Color( "green" );

    /* set timeout to restore colour */
    Glib::signal_timeout().connect( SigC::bind( SigC::slot( *this, &ContactListView::contact_restore_colour_timeout_cb ), c ), 5000 );
  }
}

void ContactListView::contact_status_change_cb(ICQ2000::StatusChangeEvent *ev)
{
  if (m_offline_contacts)
  {
    /* update contact */
    update_contact(ev->getContact());
  }
  else if (ev->getStatus() == ICQ2000::STATUS_OFFLINE)
  {
    /* keep contacts visible whilst they've got pending messages */
    if (m_message_queue.get_contact_size(ev->getContact()) == 0)
    {
      /* remove contact */
      remove_contact(ev->getContact());
    }
  }
  else
  {
    if (m_contact_map.count(ev->getContact()->getUIN()) == 0)
    {
      /* add contact */
      add_contact(ev->getContact(), icqclient.getContactTree().lookup_group_containing_contact( ev->getContact() ) );
    }
    else
    {
      /* update contact */
      update_contact(ev->getContact());
    }
  }

  sort();
}

void ContactListView::queue_added_cb(MessageEvent *ev)
{
  if (ev->getServiceType() != MessageEvent::ICQ)
    return;
  
  ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);

  if (m_contact_map.count(icq->getICQContact()->getUIN()) == 0)
  {
    /* add contact */
    add_contact(icq->getICQContact(), icqclient.getContactTree().lookup_group_containing_contact( icq->getICQContact() ) );
  }
  else
  {
    /* update contact */
    update_contact(icq->getICQContact());
  }

  sort();
}

void ContactListView::queue_removed_cb(MessageEvent *ev)
{
  if (ev->getServiceType() != MessageEvent::ICQ)
    return;
  
  ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);

  if (!m_offline_contacts
      && icq->getICQContact()->getStatus() == ICQ2000::STATUS_OFFLINE
      && m_message_queue.get_contact_size( icq->getICQContact() ) == 0)
  {
    /* remove contact */
    remove_contact(icq->getICQContact());
  }
  else
  {
    /* update contact */
    update_contact(icq->getICQContact());
  }
      
  sort();
}

  
void ContactListView::icons_changed_cb()
{
}

void ContactListView::settings_changed_cb(const std::string& key)
{
  if (key == "mouse_single_click")
  {
    m_single_click = g_settings.getValueBool(key);
  }
  else if (key == "mouse_check_away_click")
  {
    m_check_away_click = g_settings.getValueBool(key);
  }
}

void ContactListView::contact_fetch_away_msg_cb()
{
  ICQ2000::ContactRef c = get_selected_contact();
  if (c.get() != NULL
      && c->getStatus() != ICQ2000::STATUS_ONLINE
      && c->getStatus() != ICQ2000::STATUS_OFFLINE)
    icqclient.SendEvent( new ICQ2000::AwayMessageEvent(c) );
}

void ContactListView::contact_userinfo_cb()
{
}

void ContactListView::contact_send_auth_req_cb()
{
  ICQ2000::ContactRef c = get_selected_contact();
  if (c.get() != NULL && c->isICQContact())
  {
    new SendAuthReqDialog(m_parent, c);
  }
  
}

void ContactListView::contact_add_cb()
{
  new AddContactDialog(m_parent);
}

void ContactListView::contact_remove_cb()
{
  ICQ2000::ContactRef c = get_selected_contact();
  new RemoveContactDialog(m_parent, c);
}

void ContactListView::group_rename_cb()
{
  ICQ2000::ContactTree::Group * gp = get_selected_group();
  new RenameGroupDialog(m_parent, gp );
}

void ContactListView::group_remove_cb()
{
  ICQ2000::ContactTree::Group * gp = get_selected_group();
  new RemoveGroupDialog(m_parent, gp );
}

void ContactListView::group_add_cb()
{
  new AddGroupDialog(m_parent);
}

ICQ2000::ContactRef ContactListView::get_selected_contact()
{
  Glib::RefPtr<Gtk::TreeView::Selection> sel = get_selection();
  Gtk::TreeModel::iterator iter = sel->get_selected();
  Gtk::TreeModel::Row row = *iter;
  if (row[m_columns.is_contact])
  {
    ICQ2000::ContactRef c = icqclient.getContactTree().lookup_uin(row[m_columns.id]);
    return c;
  }
  else
  {
    return ICQ2000::ContactRef(NULL);
  }
}

ICQ2000::ContactTree::Group * ContactListView::get_selected_group()
{
  Glib::RefPtr<Gtk::TreeView::Selection> sel = get_selection();
  Gtk::TreeModel::iterator iter = sel->get_selected();
  Gtk::TreeModel::Row row = *iter;
  if (!row[m_columns.is_contact])
  {
    return &(icqclient.getContactTree().lookup_group(row[m_columns.id]));
  }
  else
  {
    return NULL;
  }
}

void ContactListView::set_show_offline_contacts(bool b)
{
  m_offline_contacts = b;
  update_list();
}

SigC::Signal1<void, unsigned int>& ContactListView::signal_messagebox_popup()
{
  return m_signal_messagebox_popup;
}

SigC::Signal1<void, unsigned int>& ContactListView::signal_userinfo_popup()
{
  return m_signal_userinfo_popup;
}

gint ContactListView::sort_func(const Gtk::TreeModel::iterator& iter1, const Gtk::TreeModel::iterator& iter2)
{
  Gtk::TreeModel::Row row1 = *iter1;
  Gtk::TreeModel::Row row2 = *iter2;

  int o1 = status_order (row1[m_columns.status]);
  int o2 = status_order (row2[m_columns.status]);

  int s = (o1 < o2) ? -1 : (o1 > o2) ? 1 : 0;
  int m = (row1[m_columns.messages] > row2[m_columns.messages]) ? -1 : (row1[m_columns.messages] < row2[m_columns.messages]) ? 1 : 0;
  int a = std::string(row1[m_columns.nick_sort_key]) > std::string(row2[m_columns.nick_sort_key]);

  return m ? m : s ? s : a;
}

int ContactListView::status_order (ICQ2000::Status s)
{
  switch (s)
  {
    case ICQ2000::STATUS_ONLINE:       return 1;
    case ICQ2000::STATUS_FREEFORCHAT:  return 2;
    case ICQ2000::STATUS_OCCUPIED:     return 3;
    case ICQ2000::STATUS_DND:          return 4;
    case ICQ2000::STATUS_AWAY:         return 5;
    case ICQ2000::STATUS_NA:           return 6;
    case ICQ2000::STATUS_OFFLINE:      return 7;
  }
  return 0;
}

void ContactListView::sort()
{
  /* this is a dirrty hack - gtk+ doesn't have any way of explicitly
   * resorting, and doesn't reorder rows when their contents are changed
   * (an acknowledged bug), so I hack an explicit resort function, which
   * just switches sort columns to force gtk to resort
   */
  int sort_column_id;
  Gtk::SortType order;
  
  m_reftreestore->get_sort_column_id( sort_column_id, order );
  m_reftreestore->set_sort_column_id( (sort_column_id == m_columns.nick.index()
		       ? m_columns.icon.index() : m_columns.nick.index() ), order );
}

bool ContactListView::contact_restore_weight_timeout_cb(ICQ2000::ContactRef c)
{
  /* remove highlighting on contact list */
  if ( m_contact_map.count( c->getUIN() ) > 0 )
  {
    Gtk::TreeModel::Row row = * m_contact_map[c->getUIN()];
    row[m_columns.text_weight] = Pango::WEIGHT_NORMAL;
  }

  return false;
}

bool ContactListView::contact_restore_colour_timeout_cb(ICQ2000::ContactRef c)
{
  /* remove highlighting on contact list */
  if ( m_contact_map.count( c->getUIN() ) > 0 )
  {
    Gtk::TreeModel::Row row = * m_contact_map[c->getUIN()];
    row[m_columns.text_colour] = get_style()->get_text( Gtk::STATE_NORMAL );
  }

  return false;
}
