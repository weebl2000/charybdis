/*
 * Copyright (C) 2016 Charybdis Development Team
 * Copyright (C) 2016 Jason Volk <jason@zemos.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice is present in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <ircd/m.h>

ircd::m::session::session(const host_port &host_port)
:client{host_port}
{
}

ircd::json::object
ircd::m::session::operator()(parse::buffer &pb,
                             request &r)
{
	parse::capstan pc
	{
		pb, read_closure(*this)
	};

	http::request
	{
		host(remote_addr(*this)),
		r.method,
		r.path,
		r.query,
		std::string(r),
		write_closure(*this),
		{
			{ "Content-Type"s, "application/json"s }
		}
	};

	http::code status;
	json::object object;
	http::response
	{
		pc,
		nullptr,
		[&pc, &status, &object](const http::response::head &head)
		{
			status = http::status(head.status);
			object = http::response::content{pc, head};
		}
	};

	if(status < 200 || status >= 300)
		throw m::error(status, object);

	return object;
}
