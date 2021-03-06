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
	"Client 3.4.1 :Register"
};

extern const std::string
flows;

static resource::response
post__register_guest(client &client, const resource::request::object<m::user::registar> &request);

static resource::response
post__register_user(client &client, const resource::request::object<m::user::registar> &request);

static resource::response
post__register(client &client, const resource::request::object<m::user::registar> &request);

resource
register_resource
{
	"/_matrix/client/r0/register",
	{
		"(3.3.1) Register for an account on this homeserver."
	}
};

resource::method
method_post
{
	register_resource, "POST", post__register
};

ircd::conf::item<bool>
register_enable
{
	{ "name",     "ircd.client.register.enable" },
	{ "default",  true                          }
};

/// see: ircd/m/register.h for the m::user::registar tuple.
///
resource::response
post__register(client &client,
               const resource::request::object<m::user::registar> &request)
{
	const json::object &auth
	{
		json::get<"auth"_>(request)
	};

	if(empty(auth))
		return resource::response
		{
			client, http::UNAUTHORIZED, json::object{flows}
		};

	if(!bool(register_enable))
		throw m::error
		{
			http::FORBIDDEN, "M_REGISTRATION_DISABLED",
			"Registration for this server is disabled."
		};

	const auto kind
	{
		request.query["kind"]
	};

	if(kind == "guest")
		return post__register_guest(client, request);

	if(kind.empty() || kind == "user")
		return post__register_user(client, request);

	throw m::UNSUPPORTED
	{
		"Unknown 'kind' of registration specified in query."
	};
}

ircd::conf::item<bool>
register_user_enable
{
	{ "name",     "ircd.client.register.user.enable" },
	{ "default",  true                               }
};

resource::response
post__register_user(client &client,
                    const resource::request::object<m::user::registar> &request)
try
{
	if(!bool(register_user_enable))
		throw m::error
		{
			http::FORBIDDEN, "M_REGISTRATION_DISABLED",
			"User registration for this server is disabled."
		};

	const unique_buffer<mutable_buffer> buf
	{
		4_KiB
	};

	// upcast to the user::registar tuple
	const m::user::registar &registar
	{
		request
	};

	// call operator() to register the user and receive response output
	const json::object response
	{
		registar(buf, remote(client))
	};

	// Send response to user
	return resource::response
	{
		client, http::CREATED, response
	};
}
catch(const m::INVALID_MXID &e)
{
	throw m::error
	{
		http::BAD_REQUEST, "M_INVALID_USERNAME",
		"Not a valid username. Please try again."
	};
};

ircd::conf::item<bool>
register_guest_enable
{
	{ "name",     "ircd.client.register.guest.enable" },
	{ "default",  false                               }
};

resource::response
post__register_guest(client &client,
                     const resource::request::object<m::user::registar> &request)
{
	if(!bool(register_guest_enable))
		throw m::error
		{
			http::FORBIDDEN, "M_GUEST_DISABLED",
			"Guest access is disabled"
		};

	const m::id::user::buf user_id
	{
		m::generate, my_host()
	};

	char access_token_buf[64];
	const string_view access_token
	{
		m::user::tokens::generate(access_token_buf)
	};

	return resource::response
	{
		client, http::CREATED,
		{
			{ "user_id",         user_id        },
			{ "home_server",     my_host()      },
			{ "access_token",    access_token   },
		}
	};
}

const std::string
flows{R"({
	"flows":
	[
		{
			"stages":
			[
				"m.login.dummy",
				"m.login.password",
				"m.login.email.identity"
			]
		}
	]
})"};
