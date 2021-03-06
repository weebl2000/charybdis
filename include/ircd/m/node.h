// Matrix Construct
//
// Copyright (C) Matrix Construct Developers, Authors & Contributors
// Copyright (C) 2016-2018 Jason Volk <jason@zemos.net>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice is present in all copies. The
// full license for this software is available in the LICENSE file.

#pragma once
#define HAVE_IRCD_M_NODE_H

namespace ircd::m
{
	struct node;

	extern room nodes;

	bool my(const node &);
	bool exists(const node &);
	node create(const node &, const json::members &args = {});
}

/// A node is an entity (lay: a server) participating in the matrix system. The
/// node may be a peer (ircd::server::peer) which is directly connected or it
/// may be indirect. The node may be a matrix "origin" or it may be a member
/// of a cluster perusing under the same origin.
///
/// First consider the node_id, which has the sigil ':'. A node which addresses
/// an origin as a whole has the mxid `::domain.tld` which has an empty
/// localpart. A node which is concerned with some entity within an origin has
/// an mxid `:somename:domain.tld`. This is essential for clustered multihoming
/// of our origin. Note that remote origins are supposed to be opaque, so there
/// is no real case for addressing a sub-entity other than ours.
///
struct ircd::m::node
{
	struct room;
	using key_closure = std::function<void (const string_view &)>;  // remember to unquote()!!!
	using ed25519_closure = std::function<void (const ed25519::pk &)>;

	string_view node_id;

	id::room room_id(const mutable_buffer &) const;
	id::room::buf room_id() const;

	void key(const string_view &key_id, const ed25519_closure &) const;
	void key(const string_view &key_id, const key_closure &) const;

	node(const string_view &node_id);
	node() = default;
};

/// Every node has its own room acting as a database and log mechanism
/// for this node. This is similar to the user::room.
//
struct ircd::m::node::room
:m::room
{
	m::node node;
	id::room::buf room_id;

	room(const m::node &node);
	room(const string_view &node_id);
	room() = default;
	room(const room &) = delete;
	room &operator=(const room &) = delete;
};
