// Matrix Construct
//
// Copyright (C) Matrix Construct Developers, Authors & Contributors
// Copyright (C) 2016-2018 Jason Volk <jason@zemos.net>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice is present in all copies. The
// full license for this software is available in the LICENSE file.

using namespace ircd;

mapi::header
IRCD_MODULE
{
	"Federation 20 :Device Management"
};

m::resource
user_devices_resource
{
	"/_matrix/federation/v1/user/devices",
	{
		"federation user devices",
		resource::DIRECTORY,
	}
};

static m::resource::response
get__user_devices(client &client,
                  const m::resource::request &request);

m::resource::method
method_get
{
	user_devices_resource, "GET", get__user_devices,
	{
		method_get.VERIFY_ORIGIN
	}
};

m::resource::response
get__user_devices(client &client,
                  const m::resource::request &request)
{
	if(request.parv.size() < 1)
		throw m::NEED_MORE_PARAMS
		{
			"user_id path parameter required"
		};

	m::user::id::buf user_id
	{
		url::decode(user_id, request.parv[0])
	};

	const m::user::devices user_devices
	{
		user_id
	};

	m::resource::response::chunked response
	{
		client, http::OK
	};

	json::stack out
	{
		response.buf, response.flusher()
	};

	json::stack::object top
	{
		out
	};

	json::stack::member
	{
		top, "user_id", user_id
	};

	json::stack::member
	{
		top, "stream_id", json::value
		{
			0L // unused; triggers query from synapse on m.device_list_update
		}
	};

	json::stack::array devices
	{
		top, "devices"
	};

	user_devices.for_each([&user_devices, &devices]
	(const auto &, const string_view &device_id)
	{
		json::stack::object device
		{
			devices
		};

		json::stack::member
		{
			device, "device_id", device_id
		};

		// The property name difference here is on purpose, probably one of
		// those so-called spec "thinkos"
		user_devices.get(std::nothrow, device_id, "display_name", [&device]
		(const auto &, const json::string &value)
		{
			json::stack::member
			{
				device, "device_display_name", value
			};
		});

		user_devices.get(std::nothrow, device_id, "keys", [&device]
		(const auto &, const json::object &value)
		{
			json::stack::member
			{
				device, "keys", value
			};
		});

		return true;
	});

	return std::move(response);
}
