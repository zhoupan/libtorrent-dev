/*

 Copyright (c) 2009, Arvid Norberg
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in
 the documentation and/or other materials provided with the distribution.
 * Neither the name of the author nor the names of its
 contributors may be used to endorse or promote products derived
 from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

 */

#include "libtorrent/session.hpp"
#include "libtorrent/magnet_uri.hpp"
#include "libtorrent/torrent_handle.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/bencode.hpp"

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "libtorrent.h"

int set_int_value(void* dst, int* size, int val)
{
	if (*size < sizeof(int))
		return -2;
	*((int*) dst) = val;
	*size = sizeof(int);
	return 0;
}

void copy_proxy_setting(libtorrent::proxy_settings* s, proxy_setting const* ps)
{
	s->hostname.assign(ps->hostname);
	s->port = ps->port;
	s->username.assign(ps->username);
	s->password.assign(ps->password);
	s->type = (libtorrent::proxy_settings::proxy_type) ps->type;
}

#define EXPORT extern "C" TORRENT_EXPORT

//
// session_*
//

EXPORT void* session_create(int tag, ...)
{
	va_list lp;
	va_start(lp, tag);

	libtorrent::fingerprint fing("LT", LIBTORRENT_VERSION_MAJOR, LIBTORRENT_VERSION_MINOR, 0, 0);
	std::pair<int, int> listen_range(-1, -1);
	char const* listen_interface = "0.0.0.0";
	int flags = libtorrent::session::start_default_features | libtorrent::session::add_default_plugins;
	int alert_mask = libtorrent::alert::all_categories;

	while (tag != TAG_END)
	{
		switch (tag)
		{
		case SES_FINGERPRINT:
		{
			char const* f = va_arg(lp, char const*);
			fing.name[0] = f[0];
			fing.name[1] = f[1];
			break;
		}
		case SES_LISTENPORT:
			listen_range.first = va_arg(lp, int);
			break;
		case SES_LISTENPORT_END:
			listen_range.second = va_arg(lp, int);
			break;
		case SES_VERSION_MAJOR:
			fing.major_version = va_arg(lp, int);
			break;
		case SES_VERSION_MINOR:
			fing.minor_version = va_arg(lp, int);
			break;
		case SES_VERSION_TINY:
			fing.revision_version = va_arg(lp, int);
			break;
		case SES_VERSION_TAG:
			fing.tag_version = va_arg(lp, int);
			break;
		case SES_FLAGS:
			flags = va_arg(lp, int);
			break;
		case SES_ALERT_MASK:
			alert_mask = va_arg(lp, int);
			break;
		case SES_LISTEN_INTERFACE:
			listen_interface = va_arg(lp, char const*);
			break;
		default:
			// skip unknown tags
			va_arg(lp, void*);
			break;
		}

tag	= va_arg(lp, int);
}

	if (listen_range.first != -1 && (listen_range.second == -1 || listen_range.second < listen_range.first))
		listen_range.second = listen_range.first;

	return new (std::nothrow) libtorrent::session(fing, listen_range, listen_interface, flags, alert_mask);
}

EXPORT void session_close(void* ses)
{
	delete (libtorrent::session*) ses;
}

EXPORT void* session_add_torrent(void* ses, int tag, ...)
{
	va_list lp;
	va_start(lp, tag);

	libtorrent::session* s = (libtorrent::session*) ses;
	libtorrent::add_torrent_params params;

	char const* torrent_data = 0;
	int torrent_size = 0;

	char const* resume_data = 0;
	int resume_size = 0;

	char const* magnet_url = 0;

	libtorrent::error_code ec;

	// default values
	params.paused = true;
	params.auto_managed = false;
	params.override_resume_data = true;

	while (tag != TAG_END)
	{
		switch (tag)
		{
		case TOR_FILENAME:
			params.ti = new (std::nothrow) libtorrent::torrent_info(va_arg(lp, char const*), ec);
			break;
		case TOR_TORRENT:
			torrent_data = va_arg(lp, char const*);
			break;
		case TOR_TORRENT_SIZE:
			torrent_size = va_arg(lp, int);
			break;
		case TOR_INFOHASH:
			params.ti = new (std::nothrow) libtorrent::torrent_info(libtorrent::sha1_hash(va_arg(lp, char const*)));
			break;
		case TOR_INFOHASH_HEX:
		{
			libtorrent::sha1_hash ih;
			libtorrent::from_hex(va_arg(lp, char const*), 40, (char*)&ih[0]);
			params.ti = new (std::nothrow) libtorrent::torrent_info(ih);
			break;
		}
		case TOR_MAGNETLINK:
			magnet_url = va_arg(lp, char const*);
			break;
		case TOR_TRACKER_URL:
			params.tracker_url = va_arg(lp, char const*);
			break;
		case TOR_RESUME_DATA:
			resume_data = va_arg(lp, char const*);
			break;
		case TOR_RESUME_DATA_SIZE:
			resume_size = va_arg(lp, int);
			break;
		case TOR_SAVE_PATH:
			params.save_path = va_arg(lp, char const*);
			break;
		case TOR_NAME:
			params.name = va_arg(lp, char const*);
			break;
		case TOR_PAUSED:
			params.paused = va_arg(lp, int) != 0;
			break;
		case TOR_AUTO_MANAGED:
			params.auto_managed = va_arg(lp, int) != 0;
			break;
		case TOR_DUPLICATE_IS_ERROR:
			params.duplicate_is_error = va_arg(lp, int) != 0;
			break;
		case TOR_USER_DATA:
			params.userdata = va_arg(lp, void*);
			break;
		case TOR_SEED_MODE:
			params.seed_mode = va_arg(lp, int) != 0;
			break;
		case TOR_OVERRIDE_RESUME_DATA:
			params.override_resume_data = va_arg(lp, int) != 0;
			break;
		case TOR_STORAGE_MODE:
			params.storage_mode = (libtorrent::storage_mode_t)
			va_arg(lp, int);
			break;
		default:
			// ignore unknown tags
			va_arg(lp, void*);
			break;
		};
tag	= va_arg(lp, int);
}

	if (!params.ti && torrent_data && torrent_size)
		params.ti = new (std::nothrow) libtorrent::torrent_info(torrent_data, torrent_size);

	std::vector<char> rd;
	if (resume_data && resume_size)
	{
		rd.assign(resume_data, resume_data + resume_size);
		params.resume_data = &rd;
	}
	libtorrent::torrent_handle h;
	if (!params.ti && magnet_url)
	{
		h = libtorrent::add_magnet_uri(*s, magnet_url, params, ec);
	}
	else
	{
		h = s->add_torrent(params, ec);
	}

	if (!h.is_valid())
	{
		return 0;
	}

	return new libtorrent::torrent_handle(h);
}

EXPORT void* session_add_torrent_data(void* ses, void* resume, int resume_size, const char *target)
{
	libtorrent::session* s = (libtorrent::session*) ses;
	libtorrent::add_torrent_params params;

	char const* resume_data = 0;

	libtorrent::error_code ec;

	resume_data = (char*) resume;
	params.save_path = target;

	libtorrent::lazy_entry resume_entry;
	libtorrent::lazy_bdecode(resume_data, resume_data + resume_size, resume_entry);
	std::string info_hash = resume_entry.dict_find_string_value("info-hash");
	if (info_hash.empty())
	{
		return 0;
	}

	params.info_hash = libtorrent::sha1_hash(info_hash);

	params.paused = true;
	params.auto_managed = false;
	params.override_resume_data = false;

	std::vector<char> rd;
	if (resume_data && resume_size)
	{
		rd.assign(resume_data, resume_data + resume_size);
		params.resume_data = &rd;
	}
	libtorrent::torrent_handle h;

	h = s->add_torrent(params, ec);

	if (!h.is_valid())
	{
		return 0;
	}

	return new libtorrent::torrent_handle(h);
}

EXPORT void session_remove_torrent(void* ses, void* tor, int flags)
{
	libtorrent::torrent_handle *h = (libtorrent::torrent_handle*) (tor);
	if (!h->is_valid())
		return;

	libtorrent::session* s = (libtorrent::session*) ses;
	s->remove_torrent(*h, flags);

	delete h;
}

EXPORT int session_set_settings(void* ses, int tag, ...)
{
	using namespace libtorrent;

	session* s = (session*) ses;

	va_list lp;
	va_start(lp, tag);

	while (tag != TAG_END)
	{
		switch (tag)
		{
		case SET_UPLOAD_RATE_LIMIT:
			s->set_upload_rate_limit(va_arg(lp, int));
			break;
		case SET_DOWNLOAD_RATE_LIMIT:
			s->set_download_rate_limit(va_arg(lp, int));
			break;
		case SET_LOCAL_UPLOAD_RATE_LIMIT:
			s->set_local_upload_rate_limit(va_arg(lp, int));
			break;
		case SET_LOCAL_DOWNLOAD_RATE_LIMIT:
			s->set_local_download_rate_limit(va_arg(lp, int));
			break;
		case SET_MAX_UPLOAD_SLOTS:
			s->set_max_uploads(va_arg(lp, int));
			break;
		case SET_MAX_CONNECTIONS:
			s->set_max_connections(va_arg(lp, int));
			break;
		case SET_HALF_OPEN_LIMIT:
			s->set_max_half_open_connections(va_arg(lp, int));
			break;
		case SET_PEER_PROXY:
		{
			libtorrent::proxy_settings ps;
			copy_proxy_setting(&ps, va_arg(lp, struct proxy_setting const*));
			s->set_peer_proxy(ps);
		}
		case SET_WEB_SEED_PROXY:
		{
			libtorrent::proxy_settings ps;
			copy_proxy_setting(&ps, va_arg(lp, struct proxy_setting const*));
			s->set_web_seed_proxy(ps);
		}
		case SET_TRACKER_PROXY:
		{
			libtorrent::proxy_settings ps;
			copy_proxy_setting(&ps, va_arg(lp, struct proxy_setting const*));
			s->set_tracker_proxy(ps);
		}
		case SET_ALERT_MASK:
		{
s		->set_alert_mask(va_arg(lp, int));
	}
#ifndef TORRENT_DISABLE_DHT
	case SET_DHT_PROXY:
	{
		libtorrent::proxy_settings ps;
		copy_proxy_setting(&ps, va_arg(lp, struct proxy_setting const*));
		s->set_dht_proxy(ps);
	}
#endif
	case SET_PROXY:
	{
		libtorrent::proxy_settings ps;
		copy_proxy_setting(&ps, va_arg(lp, struct proxy_setting const*));
		s->set_peer_proxy(ps);
		s->set_web_seed_proxy(ps);
		s->set_tracker_proxy(ps);
#ifndef TORRENT_DISABLE_DHT
		s->set_dht_proxy(ps);
#endif
	}
	default:
	// ignore unknown tags
	va_arg(lp, void*);
	break;
}

tag = va_arg(lp, int);
}
	return 0;
}

EXPORT int session_get_setting(void* ses, int tag, void* value, int* value_size)
{
	using namespace libtorrent;
	session* s = (session*) ses;

	switch (tag)
	{
	case SET_UPLOAD_RATE_LIMIT:
		return set_int_value(value, value_size, s->upload_rate_limit());
	case SET_DOWNLOAD_RATE_LIMIT:
		return set_int_value(value, value_size, s->download_rate_limit());
	case SET_LOCAL_UPLOAD_RATE_LIMIT:
		return set_int_value(value, value_size, s->local_upload_rate_limit());
	case SET_LOCAL_DOWNLOAD_RATE_LIMIT:
		return set_int_value(value, value_size, s->local_download_rate_limit());
	case SET_MAX_UPLOAD_SLOTS:
		return set_int_value(value, value_size, s->max_uploads());
	case SET_MAX_CONNECTIONS:
		return set_int_value(value, value_size, s->max_connections());
	case SET_HALF_OPEN_LIMIT:
		return set_int_value(value, value_size, s->max_half_open_connections());
	default:
		return -2;
	}
}

EXPORT int session_get_status(void* sesptr, struct session_status* s, int struct_size)
{
	libtorrent::session* ses = (libtorrent::session*) sesptr;

	libtorrent::session_status ss = ses->status();
	if (struct_size != sizeof(session_status))
		return -1;

	s->has_incoming_connections = ss.has_incoming_connections;

	s->upload_rate = ss.upload_rate;
	s->download_rate = ss.download_rate;
	s->total_download = ss.total_download;
	s->total_upload = ss.total_upload;

	s->payload_upload_rate = ss.payload_upload_rate;
	s->payload_download_rate = ss.payload_download_rate;
	s->total_payload_download = ss.total_payload_download;
	s->total_payload_upload = ss.total_payload_upload;

	s->ip_overhead_upload_rate = ss.ip_overhead_upload_rate;
	s->ip_overhead_download_rate = ss.ip_overhead_download_rate;
	s->total_ip_overhead_download = ss.total_ip_overhead_download;
	s->total_ip_overhead_upload = ss.total_ip_overhead_upload;

	s->dht_upload_rate = ss.dht_upload_rate;
	s->dht_download_rate = ss.dht_download_rate;
	s->total_dht_download = ss.total_dht_download;
	s->total_dht_upload = ss.total_dht_upload;

	s->tracker_upload_rate = ss.tracker_upload_rate;
	s->tracker_download_rate = ss.tracker_download_rate;
	s->total_tracker_download = ss.total_tracker_download;
	s->total_tracker_upload = ss.total_tracker_upload;

	s->total_redundant_bytes = ss.total_redundant_bytes;
	s->total_failed_bytes = ss.total_failed_bytes;

	s->num_peers = ss.num_peers;
	s->num_unchoked = ss.num_unchoked;
	s->allowed_upload_slots = ss.allowed_upload_slots;

	s->up_bandwidth_queue = ss.up_bandwidth_queue;
	s->down_bandwidth_queue = ss.down_bandwidth_queue;

	s->up_bandwidth_bytes_queue = ss.up_bandwidth_bytes_queue;
	s->down_bandwidth_bytes_queue = ss.down_bandwidth_bytes_queue;

	s->optimistic_unchoke_counter = ss.optimistic_unchoke_counter;
	s->unchoke_counter = ss.unchoke_counter;

	s->dht_nodes = ss.dht_nodes;
	s->dht_node_cache = ss.dht_node_cache;
	s->dht_torrents = ss.dht_torrents;
	s->dht_global_nodes = ss.dht_global_nodes;
	return 0;
}

EXPORT void* session_wait_alert(void* ses)
{
	libtorrent::session* s = (libtorrent::session*) ses;

	s->wait_for_alert(libtorrent::seconds(1));

	std::auto_ptr<libtorrent::alert> a = s->pop_alert();

	return a.release();
}

EXPORT void* session_pop_alert(void* ses)
{
	libtorrent::session* s = (libtorrent::session*) ses;
	std::auto_ptr<libtorrent::alert> a = s->pop_alert();

	return a.release();
}

//
// alert_*
//

EXPORT int alert_get_type(void* v)
{
	libtorrent::alert * a = (libtorrent::alert*) v;
	if (libtorrent::alert_cast<libtorrent::peer_alert>(a))
		return at_peer_alert;
	if (libtorrent::alert_cast<libtorrent::read_piece_alert>(a))
		return at_read_piece_alert;
	if (libtorrent::alert_cast<libtorrent::file_completed_alert>(a))
		return at_file_completed_alert;
	if (libtorrent::alert_cast<libtorrent::file_renamed_alert>(a))
		return at_file_renamed_alert;
	if (libtorrent::alert_cast<libtorrent::file_rename_failed_alert>(a))
		return at_file_rename_failed_alert;
	if (libtorrent::alert_cast<libtorrent::performance_alert>(a))
		return at_performance_alert;
	if (libtorrent::alert_cast<libtorrent::state_changed_alert>(a))
		return at_state_changed_alert;
	if (libtorrent::alert_cast<libtorrent::tracker_error_alert>(a))
		return at_tracker_error_alert;
	if (libtorrent::alert_cast<libtorrent::tracker_warning_alert>(a))
		return at_tracker_warning_alert;
	if (libtorrent::alert_cast<libtorrent::scrape_reply_alert>(a))
		return at_scrape_reply_alert;
	if (libtorrent::alert_cast<libtorrent::scrape_failed_alert>(a))
		return at_scrape_failed_alert;
	if (libtorrent::alert_cast<libtorrent::tracker_reply_alert>(a))
		return at_tracker_reply_alert;
	if (libtorrent::alert_cast<libtorrent::dht_reply_alert>(a))
		return at_dht_reply_alert;
	if (libtorrent::alert_cast<libtorrent::tracker_announce_alert>(a))
		return at_tracker_announce_alert;
	if (libtorrent::alert_cast<libtorrent::hash_failed_alert>(a))
		return at_hash_failed_alert;
	if (libtorrent::alert_cast<libtorrent::peer_ban_alert>(a))
		return at_peer_ban_alert;
	if (libtorrent::alert_cast<libtorrent::peer_unsnubbed_alert>(a))
		return at_peer_unsnubbed_alert;
	if (libtorrent::alert_cast<libtorrent::peer_snubbed_alert>(a))
		return at_peer_snubbed_alert;
	if (libtorrent::alert_cast<libtorrent::peer_error_alert>(a))
		return at_peer_error_alert;
	if (libtorrent::alert_cast<libtorrent::peer_connect_alert>(a))
		return at_peer_connect_alert;
	if (libtorrent::alert_cast<libtorrent::peer_disconnected_alert>(a))
		return at_peer_disconnected_alert;
	if (libtorrent::alert_cast<libtorrent::invalid_request_alert>(a))
		return at_invalid_request_alert;
	if (libtorrent::alert_cast<libtorrent::torrent_finished_alert>(a))
		return at_torrent_finished_alert;
	if (libtorrent::alert_cast<libtorrent::piece_finished_alert>(a))
		return at_piece_finished_alert;
	if (libtorrent::alert_cast<libtorrent::request_dropped_alert>(a))
		return at_request_dropped_alert;
	if (libtorrent::alert_cast<libtorrent::block_timeout_alert>(a))
		return at_block_timeout_alert;
	if (libtorrent::alert_cast<libtorrent::block_finished_alert>(a))
		return at_block_finished_alert;
	if (libtorrent::alert_cast<libtorrent::block_downloading_alert>(a))
		return at_block_downloading_alert;
	if (libtorrent::alert_cast<libtorrent::unwanted_block_alert>(a))
		return at_unwanted_block_alert;
	if (libtorrent::alert_cast<libtorrent::storage_moved_alert>(a))
		return at_storage_moved_alert;
	if (libtorrent::alert_cast<libtorrent::storage_moved_failed_alert>(a))
		return at_storage_moved_failed_alert;
	if (libtorrent::alert_cast<libtorrent::torrent_deleted_alert>(a))
		return at_torrent_deleted_alert;
	if (libtorrent::alert_cast<libtorrent::torrent_delete_failed_alert>(a))
		return at_torrent_delete_failed_alert;
	if (libtorrent::alert_cast<libtorrent::save_resume_data_alert>(a))
		return at_save_resume_data_alert;
	if (libtorrent::alert_cast<libtorrent::save_resume_data_failed_alert>(a))
		return at_save_resume_data_failed_alert;
	if (libtorrent::alert_cast<libtorrent::torrent_paused_alert>(a))
		return at_torrent_paused_alert;
	if (libtorrent::alert_cast<libtorrent::torrent_resumed_alert>(a))
		return at_torrent_resumed_alert;
	if (libtorrent::alert_cast<libtorrent::torrent_checked_alert>(a))
		return at_torrent_checked_alert;
	if (libtorrent::alert_cast<libtorrent::url_seed_alert>(a))
		return at_url_seed_alert;
	if (libtorrent::alert_cast<libtorrent::file_error_alert>(a))
		return at_file_error_alert;
	if (libtorrent::alert_cast<libtorrent::metadata_failed_alert>(a))
		return at_metadata_failed_alert;
	if (libtorrent::alert_cast<libtorrent::metadata_received_alert>(a))
		return at_metadata_received_alert;
	if (libtorrent::alert_cast<libtorrent::udp_error_alert>(a))
		return at_udp_error_alert;
	if (libtorrent::alert_cast<libtorrent::external_ip_alert>(a))
		return at_external_ip_alert;
	if (libtorrent::alert_cast<libtorrent::listen_failed_alert>(a))
		return at_listen_failed_alert;
	if (libtorrent::alert_cast<libtorrent::listen_succeeded_alert>(a))
		return at_listen_succeeded_alert;
	if (libtorrent::alert_cast<libtorrent::portmap_error_alert>(a))
		return at_portmap_error_alert;
	if (libtorrent::alert_cast<libtorrent::portmap_alert>(a))
		return at_portmap_alert;
	if (libtorrent::alert_cast<libtorrent::portmap_log_alert>(a))
		return at_portmap_log_alert;
	if (libtorrent::alert_cast<libtorrent::fastresume_rejected_alert>(a))
		return at_fastresume_rejected_alert;
	if (libtorrent::alert_cast<libtorrent::peer_blocked_alert>(a))
		return at_peer_blocked_alert;
	if (libtorrent::alert_cast<libtorrent::dht_announce_alert>(a))
		return at_dht_announce_alert;
	if (libtorrent::alert_cast<libtorrent::dht_get_peers_alert>(a))
		return at_dht_get_peers_alert;
	if (libtorrent::alert_cast<libtorrent::stats_alert>(a))
		return at_stats_alert;
	if (libtorrent::alert_cast<libtorrent::cache_flushed_alert>(a))
		return at_cache_flushed_alert;
	return at_alert;
}

void copy_alert(libtorrent::alert *a, alert& aa)
{
	aa.timestamp = a->timestamp().time;
	aa.message = new char[a->message().length() + 1];
	strcpy(aa.message, a->message().c_str());
	aa.what = new char[strlen(a->what()) + 1];
	strcpy(aa.what, a->what());
	aa.category = a->category();
}

void free_alert(alert& aa)
{
	delete aa.message;
	delete aa.what;
}

void copy_torrent_alert(libtorrent::torrent_alert* a, torrent_alert& aa)
{
	copy_alert(a, aa.alert);

	aa.handle = new libtorrent::torrent_handle(a->handle);
}

void free_torrent_alert(torrent_alert& aa)
{
	delete (libtorrent::torrent_handle*) aa.handle;
}

char* dup_string(std::string str)
{
	char* ret = new char[str.length() + 1];

	strcpy(ret, str.c_str());

	return ret;
}

void copy_tracker_alert(libtorrent::tracker_alert *a, tracker_alert& aa)
{
	copy_torrent_alert(a, aa.torrent_alert);

	aa.url = dup_string(a->url);
}

void free_tracker_alert(tracker_alert& tracker_alert)
{
	delete tracker_alert.url;
}

void copy_error_code(boost::system::error_code &a, error_code &aa)
{
	aa.value = a.value();
	aa.category = dup_string(a.category().name());
	aa.message = dup_string(a.message());
}

void free_error_code(error_code &aa)
{
	delete aa.category;
	delete aa.message;
}

void copy_endpoint(boost::asio::ip::tcp::endpoint &a, endpoint& endpoint)
{
	boost::system::error_code ec;

	std::string str = a.address().to_string(ec);
	endpoint.addr = new char[str.length() + 1];
	strcpy((char*) endpoint.addr, str.c_str());

	endpoint.port = a.port();

	copy_error_code(ec, endpoint.error);
}

void free_endpoint(endpoint& endpoint)
{
	free_error_code(endpoint.error);

	delete endpoint.addr;
}

EXPORT void session_free_alert(void *pop)
{
	libtorrent::alert * a = (libtorrent::alert*) pop;
	delete a;
}

EXPORT listen_succeeded_alert * cast_to_listen_succeeded_alert(void* pop)
{
	listen_succeeded_alert * to = new listen_succeeded_alert();
	libtorrent::listen_succeeded_alert * a = (libtorrent::listen_succeeded_alert*) pop;

	copy_alert(a, to->alert);
	copy_endpoint(a->endpoint, to->endpoint);

	session_free_alert(a);

	return to;
}

EXPORT void free_listen_succeeded_alert(listen_succeeded_alert * listen_succeeded_alert)
{
	free_alert(listen_succeeded_alert->alert);
	free_endpoint(listen_succeeded_alert->endpoint);

	delete listen_succeeded_alert;
}

EXPORT torrent_paused_alert * cast_to_torrent_paused_alert(void* pop)
{
	libtorrent::torrent_paused_alert * a = (libtorrent::torrent_paused_alert*) pop;
	torrent_paused_alert* aa = new torrent_paused_alert();
	copy_torrent_alert(a, aa->torrent_alert);

	session_free_alert(a);

	return aa;
}

EXPORT void free_torrent_paused_alert(torrent_paused_alert * aa)
{
	free_torrent_alert(aa->torrent_alert);
	delete aa;
}

EXPORT torrent_checked_alert * cast_to_torrent_checked_alert(void* pop)
{
	libtorrent::torrent_checked_alert * a = (libtorrent::torrent_checked_alert*) pop;
	torrent_checked_alert* aa = new torrent_checked_alert();
	copy_torrent_alert(a, aa->torrent_alert);

	session_free_alert(a);

	return aa;
}

EXPORT void free_torrent_checked_alert(torrent_checked_alert * aa)
{
	free_torrent_alert(aa->torrent_alert);
	delete aa;
}

//
// torrent_*
//

EXPORT int torrent_get_status(void* tor, torrent_status* s, int struct_size)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	libtorrent::torrent_status ts = h.status();

	if (struct_size != sizeof(torrent_status))
		return -1;

	s->state = (state_t) ts.state;
	s->paused = ts.paused;
	s->progress = ts.progress;
	strncpy(s->error, ts.error.c_str(), 1025);
	s->next_announce = ts.next_announce.total_seconds();
	s->announce_interval = ts.announce_interval.total_seconds();
	strncpy(s->current_tracker, ts.current_tracker.c_str(), 512);
	s->total_download = ts.total_download = ts.total_download = ts.total_download;
	s->total_upload = ts.total_upload = ts.total_upload = ts.total_upload;
	s->total_payload_download = ts.total_payload_download;
	s->total_payload_upload = ts.total_payload_upload;
	s->total_failed_bytes = ts.total_failed_bytes;
	s->total_redundant_bytes = ts.total_redundant_bytes;
	s->download_rate = ts.download_rate;
	s->upload_rate = ts.upload_rate;
	s->download_payload_rate = ts.download_payload_rate;
	s->upload_payload_rate = ts.upload_payload_rate;
	s->num_seeds = ts.num_seeds;
	s->num_peers = ts.num_peers;
	s->num_complete = ts.num_complete;
	s->num_incomplete = ts.num_incomplete;
	s->list_seeds = ts.list_seeds;
	s->list_peers = ts.list_peers;
	s->connect_candidates = ts.connect_candidates;
	s->num_pieces = ts.num_pieces;
	s->total_done = ts.total_done;
	s->total_wanted_done = ts.total_wanted_done;
	s->total_wanted = ts.total_wanted;
	s->distributed_copies = ts.distributed_copies;
	s->block_size = ts.block_size;
	s->num_uploads = ts.num_uploads;
	s->num_connections = ts.num_connections;
	s->uploads_limit = ts.uploads_limit;
	s->connections_limit = ts.connections_limit;
//	s->storage_mode = (storage_mode_t)ts.storage_mode;
	s->up_bandwidth_queue = ts.up_bandwidth_queue;
	s->down_bandwidth_queue = ts.down_bandwidth_queue;
	s->all_time_upload = ts.all_time_upload;
	s->all_time_download = ts.all_time_download;
	s->active_time = ts.active_time;
	s->seeding_time = ts.seeding_time;
	s->seed_rank = ts.seed_rank;
	s->last_scrape = ts.last_scrape;
	s->has_incoming = ts.has_incoming;
	s->sparse_regions = ts.sparse_regions;
	s->seed_mode = ts.seed_mode;
	return 0;
}

EXPORT int torrent_set_settings(void * tor, int tag, ...)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	va_list lp;
	va_start(lp, tag);

	while (tag != TAG_END)
	{
		switch (tag)
		{
		case SET_UPLOAD_RATE_LIMIT:
			h.set_upload_limit(va_arg(lp, int));
			break;
		case SET_DOWNLOAD_RATE_LIMIT:
			h.set_download_limit(va_arg(lp, int));
			break;
		case SET_MAX_UPLOAD_SLOTS:
			h.set_max_uploads(va_arg(lp, int));
			break;
		case SET_MAX_CONNECTIONS:
			h.set_max_connections(va_arg(lp, int));
			break;
		case SET_SEQUENTIAL_DOWNLOAD:
			h.set_sequential_download(va_arg(lp, int) != 0);
			break;
		case SET_SUPER_SEEDING:
			h.super_seeding(va_arg(lp, int) != 0);
			break;
		default:
			// ignore unknown tags
			va_arg(lp, void*);
			break;
		}

tag	= va_arg(lp, int);
}
	return 0;
}

EXPORT int torrent_get_setting(void *tor, int tag, void* value, int* value_size)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	switch (tag)
	{
	case SET_UPLOAD_RATE_LIMIT:
		return set_int_value(value, value_size, h.upload_limit());
	case SET_DOWNLOAD_RATE_LIMIT:
		return set_int_value(value, value_size, h.download_limit());
	case SET_MAX_UPLOAD_SLOTS:
		return set_int_value(value, value_size, h.max_uploads());
	case SET_MAX_CONNECTIONS:
		return set_int_value(value, value_size, h.max_connections());
	case SET_SEQUENTIAL_DOWNLOAD:
		return set_int_value(value, value_size, h.is_sequential_download());
	case SET_SUPER_SEEDING:
		return set_int_value(value, value_size, h.super_seeding());
	default:
		return -2;
	}
}

EXPORT int torrent_pause(void *tor)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	h.pause();

	return 0;
}

EXPORT long torrent_get_hash(void *tor)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	return libtorrent::hash_value(h.status(0));
}

EXPORT int torrent_resume(void* tor)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	h.resume();
	return 0;
}

EXPORT int torrent_recheck(void* tor)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	h.force_recheck();
	return 0;
}

EXPORT int torrent_save_state(void *tor)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	h.save_resume_data();

	return 0;
}

EXPORT bool torrent_has_metadata(void*tor)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return false;

	return h.has_metadata();
}

EXPORT void* torrent_get_metadata(void*tor)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return 0;

	const libtorrent::torrent_info& t = h.get_torrent_info();
	boost::shared_array<char> m = t.metadata();
	return m.get();
}

EXPORT int torrent_get_metadata_size(void*tor)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	const libtorrent::torrent_info& t = h.get_torrent_info();

	return t.metadata_size();
}

EXPORT torrent_resumed_alert * cast_to_torrent_resumed_alert(void* pop)
{
	libtorrent::torrent_resumed_alert * a = (libtorrent::torrent_resumed_alert*) pop;
	torrent_resumed_alert* aa = new torrent_resumed_alert();
	copy_torrent_alert(a, aa->torrent_alert);

	session_free_alert(a);

	return aa;
}

EXPORT void free_torrent_resumed_alert(torrent_resumed_alert * aa)
{
	free_torrent_alert(aa->torrent_alert);
	delete aa;
}

EXPORT save_resume_data_alert * cast_to_save_resume_data_alert(void *pop)
{
	libtorrent::save_resume_data_alert * a = (libtorrent::save_resume_data_alert*) pop;
	save_resume_data_alert * aa = new save_resume_data_alert();

	copy_torrent_alert(a, aa->torrent_alert);

	std::vector<char> out;

	libtorrent::bencode(std::back_inserter(out), *a->resume_data);

	aa->resume_data = new char[out.size()];
	aa->resume_data_size = out.size();

	std::copy(out.begin(), out.end(), aa->resume_data);

	session_free_alert(a);

	return aa;
}

EXPORT void free_save_resume_data_alert(save_resume_data_alert *aa)
{
	free_torrent_alert(aa->torrent_alert);

	delete aa->resume_data;

	delete aa;
}

EXPORT save_resume_data_failed_alert* cast_to_save_resume_data_failed_alert(void *pop)
{
	libtorrent::save_resume_data_failed_alert * a = (libtorrent::save_resume_data_failed_alert*) pop;
	save_resume_data_failed_alert* aa = new save_resume_data_failed_alert();

	copy_error_code(a->error, aa->error);

	session_free_alert(a);

	return aa;
}

EXPORT void free_save_resume_data_failed_alert(save_resume_data_failed_alert* aa)
{
	free_error_code(aa->error);

	delete aa;
}

EXPORT int torrent_files_count(void* tor)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	const libtorrent::torrent_info& t = h.get_torrent_info();

	libtorrent::file_storage const& fs = t.files();
	return fs.num_files();
}

EXPORT int torrent_files_get(void* tor, int i, struct file_entry* fe)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	const libtorrent::torrent_info& t = h.get_torrent_info();

	const libtorrent::file_storage &fs = t.files();

	const libtorrent::file_entry &f = fs.at(i);

	fe->path = dup_string(f.path);
	fe->size = f.size;
	fe->mtime = f.mtime;
	fe->pad_file = f.pad_file;
	fe->hidden_attribute = f.hidden_attribute;
	fe->executable_attribute = f.executable_attribute;
	fe->symlink_attribute = f.symlink_attribute;
	fe->symlink_path = dup_string(f.symlink_path);

	return 0;
}

EXPORT int torrent_files_free(void* tor, struct file_entry* f)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	const libtorrent::torrent_info& t = h.get_torrent_info();

	delete f->path;
	delete f->symlink_path;

	return 0;
}

EXPORT int torrent_files_rename(void* tor, int i, const char* fe)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	h.rename_file(i, std::string(fe));

	return 0;
}

EXPORT int torrent_info(void* tor, torrent_title* tt)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	libtorrent::torrent_status s = h.status();

	if (s.has_metadata)
	{
		const libtorrent::torrent_info& t = h.get_torrent_info();

		tt->name = dup_string(t.name());
		tt->comment = dup_string(t.comment());
		tt->creator = dup_string(t.creator());

		tt->mtime = *t.creation_date();
	}
	else
	{
		tt->name = dup_string(h.name());
	}

	return 0;
}

EXPORT int torrent_info_free(void* tor, torrent_title* tt)
{
	libtorrent::torrent_handle &h = *(libtorrent::torrent_handle*) (tor);
	if (!h.is_valid())
		return -1;

	if (tt->name != 0)
		delete tt->name;
	if (tt->comment != 0)
		delete tt->comment;
	if (tt->creator != 0)
		delete tt->creator;

	return 0;
}
