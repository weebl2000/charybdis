// Matrix Construct
//
// Copyright (C) Matrix Construct Developers, Authors & Contributors
// Copyright (C) 2016-2018 Jason Volk <jason@zemos.net>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice is present in all copies. The
// full license for this software is available in the LICENSE file.

ircd::mapi::header
IRCD_MODULE
{
	"Client Sync :Room State"
};

namespace ircd::m::sync
{
	static void room_state_polylog_events(data &);
	static void room_state_polylog(data &);

	static void room_state_linear_events(data &);
	static void room_state_linear(data &);

	extern const event::keys::include _default_keys;
	extern item room_state;
}

decltype(ircd::m::sync::room_state)
ircd::m::sync::room_state
{
	"rooms.state",
	room_state_polylog,
	room_state_linear
};

decltype(ircd::m::sync::_default_keys)
ircd::m::sync::_default_keys
{
	"content",
	"depth",
	"event_id",
	"origin_server_ts",
	"redacts",
	"room_id",
	"sender",
	"state_key",
	"type",
};

void
ircd::m::sync::room_state_linear(data &data)
{
	assert(data.event);
	assert(data.room);
	assert(json::get<"room_id"_>(*data.event));

	if(!json::get<"state_key"_>(*data.event))
		return;

	if(!data.room->membership(data.user, data.membership))
		return;

	//data.array->append(*data.event);
}

void
ircd::m::sync::room_state_polylog(data &data)
{
	json::stack::object object
	{
		data.out
	};

	room_state_polylog_events(data);
}

void
ircd::m::sync::room_state_polylog_events(data &data)
{
	json::stack::array array
	{
		data.out, "events"
	};

	ctx::mutex mutex;
	const event::closure_idx each_idx{[&data, &array, &mutex]
	(const m::event::idx &event_idx)
	{
		static const m::event::fetch::opts fopts
		{
			_default_keys
		};

		const event::fetch event
		{
			event_idx, std::nothrow, fopts
		};

		if(!event.valid)
			return;

		const std::lock_guard<decltype(mutex)> lock{mutex};
		data.commit();
		array.append(event);
	}};

	//TODO: conf
	std::array<event::idx, 16> md;
	ctx::parallel<event::idx> parallel
	{
		m::sync::pool, md, each_idx
	};

	const m::room &room{*data.room};
	const m::room::state state{room};
	state.for_each([&data, &parallel]
	(const m::event::idx &event_idx)
	{
		if(apropos(data, event_idx))
			parallel(event_idx);
	});
}