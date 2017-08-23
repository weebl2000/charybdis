/*
 * charybdis: 21st Century IRC++d
 *
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
 *
 */

#pragma once
#define HAVE_IRCD_M_EVENT_H

///////////////////////////////////////////////////////////////////////////////
// Protocol notes
//
// 10.4
// The total size of any event MUST NOT exceed 65 KB.
//
// There are additional restrictions on sizes per key:
// sender MUST NOT exceed 255 bytes (including domain).
// room_id MUST NOT exceed 255 bytes.
// state_key MUST NOT exceed 255 bytes.
// type MUST NOT exceed 255 bytes.
// event_id MUST NOT exceed 255 bytes.
//
// Some event types have additional size restrictions which are specified in
// the description of the event. Additional keys have no limit other than that
// implied by the total 65 KB limit on events.
//

namespace ircd {
namespace m    {

struct event
:json::parse
<
	string_view,
	time_t,
	string_view,
	string_view,
	string_view,
	string_view
>
{
	IRCD_MEMBERS
	(
		"content",
		"origin_server_ts",
		"sender",
		"type",
		"unsigned",
		"state_key"
	)

	template<class... A>
	event(A&&... a)
	:parse{*this, std::forward<A>(a)...}
	{}
};

} // namespace m
} // namespace ircd
