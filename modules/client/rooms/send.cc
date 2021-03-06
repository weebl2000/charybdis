// Matrix Construct
//
// Copyright (C) Matrix Construct Developers, Authors & Contributors
// Copyright (C) 2016-2018 Jason Volk <jason@zemos.net>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice is present in all copies. The
// full license for this software is available in the LICENSE file.

#include "rooms.h"

using namespace ircd::m;
using namespace ircd;

static bool
check_transaction_id(const m::user::id &user_id,
                     const string_view &transaction_id);

static void
save_transaction_id(const m::event &,
                    m::vm::eval &);

static m::resource::response
handle_command(client &,
               const m::resource::request &,
               const room &);

m::hookfn<m::vm::eval &>
save_transaction_id_hookfn
{
	save_transaction_id,
	{
		{ "_site",    "vm.post" },
		{ "origin",   my_host() },
	}
};

m::resource::response
put__send(client &client,
          const m::resource::request &request,
          const room::id &room_id)
{
	if(request.parv.size() < 3)
		throw NEED_MORE_PARAMS
		{
			"type parameter missing"
		};

	char type_buf[m::event::TYPE_MAX_SIZE];
	const string_view &type
	{
		url::decode(type_buf, request.parv[2])
	};

	if(request.parv.size() < 4)
		throw NEED_MORE_PARAMS
		{
			"txnid parameter missing"
		};

	char transaction_id_buf[64];
	const string_view &transaction_id
	{
		url::decode(transaction_id_buf, request.parv[3])
	};

	if(!check_transaction_id(request.user_id, transaction_id))
		throw m::error
		{
			http::CONFLICT, "M_DUPLICATE_TXNID",
			"Already processed request with txnid '%s'",
			transaction_id
		};

	m::vm::copts copts;
	copts.client_txnid = transaction_id;
	const room room
	{
		room_id, &copts
	};

	const json::object &content
	{
		request
	};

	const bool cmd
	{
		type == "m.room.message" &&
		unquote(content.get("msgtype")) == "m.text" &&
		startswith(unquote(content.get("body")), "\\\\")
	};

	if(cmd)
		return handle_command(client, request, room);

	const auto event_id
	{
		m::send(room, request.user_id, type, content)
	};

	return m::resource::response
	{
		client, json::members
		{
			{ "event_id", event_id }
		}
	};
}

m::resource::response
handle_command(client &client,
               const m::resource::request &request,
               const room &room)
{
	const user::room user_room
	{
		request.user_id, room.copts
	};

	const auto event_id
	{
		send(user_room, request.user_id, "ircd.cmd",
		{
			{ "msgtype",  "m.text"         },
			{ "body",     request["body"]  },
			{ "room_id",  room.room_id     },
		})
	};

	return m::resource::response
	{
		client, json::members
		{
			{ "event_id",  event_id },
			{ "cmd",       true     },
		}
	};
}

void
save_transaction_id(const m::event &event,
                    m::vm::eval &eval)
{
	if(!eval.copts)
		return;

	if(!eval.copts->client_txnid)
		return;

	if(!event.event_id)
		return;

	assert(my_host(at<"origin"_>(event)));
	const m::user::room user_room
	{
		at<"sender"_>(event)
	};

	static const string_view &type
	{
		"ircd.client.txnid"
	};

	const auto &state_key
	{
		event.event_id
	};

	send(user_room, at<"sender"_>(event), type, state_key,
	{
		{ "transaction_id", eval.copts->client_txnid }
	});
}

// Using a linear search here because we have no index on txnids as this
// is the only codepath where we'd perform that lookup; in contrast the
// event_id -> txnid query is made far more often for client sync.
//
// This means we have to set some arbitrary limits on the linear search:
// lim[0] is a total limit of events to iterate, so if the user's room
// has a lot of activity we might return a false non-match and allow a
// duplicate txnid; this is highly unlikely. lim[1] allows the user to
// have several /sends in flight at the same time, also unlikely but we
// avoid that case for false non-match.
static bool
check_transaction_id(const m::user::id &user_id,
                     const string_view &transaction_id)
{
	static const auto type_match
	{
		[](const string_view &type)
		{
			return type == "ircd.client.txnid";
		}
	};

	const auto content_match
	{
		[&transaction_id](const json::object &content)
		{
			const json::string &value
			{
				content["transaction_id"]
			};

			return value == transaction_id;
		}
	};

	ssize_t lim[] { 128, 3 };
	const m::user::room user_room{user_id};
	for(m::room::events it(user_room); it && lim[0] > 0 && lim[1] > 0; --it, --lim[0])
	{
		if(!m::query(std::nothrow, it.event_idx(), "type", type_match))
			continue;

		--lim[1];
		if(!m::query(std::nothrow, it.event_idx(), "content", content_match))
			continue;

		return false;
	}

	return true;
}
