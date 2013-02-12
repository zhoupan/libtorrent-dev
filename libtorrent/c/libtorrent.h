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

#ifndef LIBTORRENT_H
#define LIBTORRENT_H

enum tags
{
	TAG_END = 0,

	SES_FINGERPRINT, // char const*, 2 character string
	SES_LISTENPORT, // int
	SES_LISTENPORT_END, // int
	SES_VERSION_MAJOR, // int
	SES_VERSION_MINOR, // int
	SES_VERSION_TINY, // int
	SES_VERSION_TAG, // int
	SES_FLAGS, // int
	SES_ALERT_MASK, // int
	SES_LISTEN_INTERFACE, // char const*

	// === add_torrent tags ===

	// identifying the torrent to add
	TOR_FILENAME = 0x100, // char const*
	TOR_TORRENT, // char const*, specify size of buffer with TOR_TORRENT_SIZE
	TOR_TORRENT_SIZE, // int
	TOR_INFOHASH, // char const*, must point to a 20 byte array
	TOR_INFOHASH_HEX, // char const*, must point to a 40 byte string
	TOR_MAGNETLINK, // char const*, url

	TOR_TRACKER_URL, // char const*
	TOR_RESUME_DATA, // char const*
	TOR_RESUME_DATA_SIZE, // int
	TOR_SAVE_PATH, // char const*
	TOR_NAME, // char const*
	TOR_PAUSED, // int
	TOR_AUTO_MANAGED, // int
	TOR_DUPLICATE_IS_ERROR, // int
	TOR_USER_DATA, //void*
	TOR_SEED_MODE, // int
	TOR_OVERRIDE_RESUME_DATA, // int
	TOR_STORAGE_MODE, // int

	SET_UPLOAD_RATE_LIMIT = 0x200, // int
	SET_DOWNLOAD_RATE_LIMIT, // int
	SET_LOCAL_UPLOAD_RATE_LIMIT, // int
	SET_LOCAL_DOWNLOAD_RATE_LIMIT, // int
	SET_MAX_UPLOAD_SLOTS, // int
	SET_MAX_CONNECTIONS, // int
	SET_SEQUENTIAL_DOWNLOAD, // int, torrent only
	SET_SUPER_SEEDING, // int, torrent only
	SET_HALF_OPEN_LIMIT, // int, session only
	SET_PEER_PROXY, // proxy_setting const*, session_only
	SET_WEB_SEED_PROXY, // proxy_setting const*, session_only
	SET_TRACKER_PROXY, // proxy_setting const*, session_only
	SET_DHT_PROXY, // proxy_setting const*, session_only
	SET_PROXY, // proxy_setting const*, session_only
	SET_ALERT_MASK,
// int, session_only
};

struct proxy_setting
{
	char hostname[256];
	int port;

	char username[256];
	char password[256];

	int type;
};

enum category_t
{
	cat_error = 0x1,
	cat_peer = 0x2,
	cat_port_mapping = 0x4,
	cat_storage = 0x8,
	cat_tracker = 0x10,
	cat_debug = 0x20,
	cat_status = 0x40,
	cat_progress = 0x80,
	cat_ip_block = 0x100,
	cat_performance_warning = 0x200,
	cat_dht = 0x400,

	cat_all_categories = 0xffffffff
};

enum proxy_type_t
{
	proxy_none, proxy_socks4, proxy_socks5, proxy_socks5_pw, proxy_http, proxy_http_pw
};

enum storage_mode_t
{
	storage_mode_allocate = 0, storage_mode_sparse, storage_mode_compact
};

enum state_t
{
	queued_for_checking,
	checking_files,
	downloading_metadata,
	downloading,
	finished,
	seeding,
	allocating,
	checking_resume_data
};

struct torrent_status
{
	enum state_t state;
	int paused;
	float progress;
	char error[1024];
	int next_announce;
	int announce_interval;
	char current_tracker[512];
	long long total_download;
	long long total_upload;
	long long total_payload_download;
	long long total_payload_upload;
	long long total_failed_bytes;
	long long total_redundant_bytes;
	float download_rate;
	float upload_rate;
	float download_payload_rate;
	float upload_payload_rate;
	int num_seeds;
	int num_peers;
	int num_complete;
	int num_incomplete;
	int list_seeds;
	int list_peers;
	int connect_candidates;

	// what to do?
//	bitfield pieces;

	int num_pieces;
	long long total_done;
	long long total_wanted_done;
	long long total_wanted;
	float distributed_copies;
	int block_size;
	int num_uploads;
	int num_connections;
	int uploads_limit;
	int connections_limit;
//	enum storage_mode_t storage_mode;
	int up_bandwidth_queue;
	int down_bandwidth_queue;
	long long all_time_upload;
	long long all_time_download;
	int active_time;
	int seeding_time;
	int seed_rank;
	int last_scrape;
	int has_incoming;
	int sparse_regions;
	int seed_mode;
};

struct session_status
{
	int has_incoming_connections;

	float upload_rate;
	float download_rate;
	long long total_download;
	long long total_upload;

	float payload_upload_rate;
	float payload_download_rate;
	long long total_payload_download;
	long long total_payload_upload;

	float ip_overhead_upload_rate;
	float ip_overhead_download_rate;
	long long total_ip_overhead_download;
	long long total_ip_overhead_upload;

	float dht_upload_rate;
	float dht_download_rate;
	long long total_dht_download;
	long long total_dht_upload;

	float tracker_upload_rate;
	float tracker_download_rate;
	long long total_tracker_download;
	long long total_tracker_upload;

	long long total_redundant_bytes;
	long long total_failed_bytes;

	int num_peers;
	int num_unchoked;
	int allowed_upload_slots;

	int up_bandwidth_queue;
	int down_bandwidth_queue;

	int up_bandwidth_bytes_queue;
	int down_bandwidth_bytes_queue;

	int optimistic_unchoke_counter;
	int unchoke_counter;

	int dht_nodes;
	int dht_node_cache;
	int dht_torrents;
	long long dht_global_nodes;
//	std::vector<dht_lookup> active_requests;
};

// alerts

enum alert_type
{
	at_torrent_alert,
	at_peer_alert,
	at_tracker_alert,
	at_read_piece_alert,
	at_file_completed_alert,
	at_file_renamed_alert,
	at_file_rename_failed_alert,
	at_performance_alert,
	at_state_changed_alert,
	at_tracker_error_alert,
	at_tracker_warning_alert,
	at_scrape_reply_alert,
	at_scrape_failed_alert,
	at_tracker_reply_alert,
	at_dht_reply_alert,
	at_tracker_announce_alert,
	at_hash_failed_alert,
	at_peer_ban_alert,
	at_peer_unsnubbed_alert,
	at_peer_snubbed_alert,
	at_peer_error_alert,
	at_peer_connect_alert,
	at_peer_disconnected_alert,
	at_invalid_request_alert,
	at_torrent_finished_alert,
	at_piece_finished_alert,
	at_request_dropped_alert,
	at_block_timeout_alert,
	at_block_finished_alert,
	at_block_downloading_alert,
	at_unwanted_block_alert,
	at_storage_moved_alert,
	at_storage_moved_failed_alert,
	at_torrent_deleted_alert,
	at_torrent_delete_failed_alert,
	at_save_resume_data_alert,
	at_save_resume_data_failed_alert,
	at_torrent_paused_alert,
	at_torrent_resumed_alert,
	at_torrent_checked_alert,
	at_url_seed_alert,
	at_file_error_alert,
	at_metadata_failed_alert,
	at_metadata_received_alert,
	at_udp_error_alert,
	at_external_ip_alert,
	at_listen_failed_alert,
	at_listen_succeeded_alert,
	at_portmap_error_alert,
	at_portmap_alert,
	at_portmap_log_alert,
	at_fastresume_rejected_alert,
	at_peer_blocked_alert,
	at_dht_announce_alert,
	at_dht_get_peers_alert,
	at_stats_alert,
	at_cache_flushed_alert,
	at_alert
};

//
// part structures
//

struct error_code
{
	int value;
	char* category;
	char * message;
};

struct endpoint
{
	const char* addr;

	int port;

	struct error_code error;
};

struct peer_request
{
	int piece;
	int start;
	int length;
};

//
// alert structures
//

struct alert
{
	int timestamp;
	char *message;
	char *what;
	int category;
};

struct torrent_alert
{
	struct alert alert;

	void* handle;
};

struct tracker_alert
{
	struct torrent_alert torrent_alert;

	char* url;
};

struct read_piece_alert
{
	struct torrent_alert torrent_alert;

	void* buffer;
	int piece;
	int size;
};

struct external_ip_alert
{
	struct alert alert;

	int external_address;
};

struct listen_failed_alert
{
	struct alert alert;

	struct endpoint endpoint;
	struct error_code error;
};

struct listen_succeeded_alert
{
	struct alert alert;

	struct endpoint endpoint;
};

struct portmap_error_alert
{
	struct alert alert;

	int mapping;
	int type;
	struct error_code error;
};

struct portmap_alert
{
	struct alert alert;

	int mapping;
	int external_port;
	int type;
};

struct portmap_log_alert
{
	struct alert alert;

	int type;
	const char* msg;
};

struct file_error_alert
{
	struct torrent_alert torrent_alert;

	const char* file;
	struct error_code error;
};

struct file_renamed_alert
{
	struct torrent_alert torrent_alert;

	const char* name;
	int index;
};

struct file_rename_failed_alert
{
	struct torrent_alert torrent_alert;

	int index;
	struct error_code error;
};

struct tracker_announce_alert
{
	struct tracker_alert tracker_alert;

	int event;
};

struct tracker_error_alert
{
	struct tracker_alert tracker_alert;

	int times_in_row;
	int status_code;
};

struct tracker_reply_alert
{
	struct tracker_alert tracker_alert;

	int num_peers;
};

struct dht_reply_alert
{
	struct tracker_alert tracker_alert;

	int num_peers;
};

struct tracker_warning_alert
{
	struct tracker_alert tracker_alert;

	const char* msg;
};

struct scrape_reply_alert
{
	struct tracker_alert tracker_alert;

	int incomplete;
	int complete;
};

struct scrape_failed_alert
{
	struct tracker_alert tracker_alert;

	const char * msg;
};

struct url_seed_alert
{
	struct torrent_alert torrent_alert;

	const char * url;
};

struct hash_failed_alert
{
	struct torrent_alert torrent_alert;

	int piece_index;
};

struct peer_alert
{
	struct torrent_alert torrent_alert;

	struct endpoint ip;
	int pid;
};

struct peer_connect_alert
{
	struct peer_alert peer_alert;
};

struct peer_ban_alert
{
	struct peer_alert peer_alert;

};

struct peer_snubbed_alert
{
	struct peer_alert peer_alert;
};

struct peer_unsnubbed_alert
{
	struct peer_alert peer_alert;
};

struct peer_error_alert
{
	struct peer_alert peer_alert;
	struct error_code error;
};

struct peer_disconnected_alert
{
	struct peer_alert peer_alert;

	struct error_code error;
};

struct invalid_request_alert
{
	struct peer_alert peer_alert;

	struct peer_request request;
};

struct request_dropped_alert
{
	struct peer_alert peer_alert;

	int block_index;
	int piece_index;
};

struct block_timeout_alert
{
	struct peer_alert peer_alert;

	int block_index;
	int piece_index;
};

struct block_finished_alert
{
	struct peer_alert peer_alert;

	int block_index;
	int piece_index;
};

struct file_completed_alert
{
	struct torrent_alert torrent_alert;

	int index;
};

struct block_downloading_alert
{
	struct peer_alert peer_alert;

	int block_index;
	int piece_index;
};

struct unwanted_block_alert
{
	struct peer_alert peer_alert;

	int block_index;
	int piece_index;
};

struct torrent_delete_failed_alert
{
	struct torrent_alert torrent_alert;

	struct error_code error;
};

struct torrent_deleted_alert
{
	struct torrent_alert torrent_alert;

	const char* info_hash;
};

struct performance_alert
{
	struct torrent_alert torrent_alert;

	enum performance_warning_t
	{
		outstanding_disk_buffer_limit_reached,
		outstanding_request_limit_reached,
		upload_limit_too_low,
		download_limit_too_low,
		send_buffer_watermark_too_low
	};

	enum performance_warning_t warning_code;
};

struct state_changed_alert
{
	struct torrent_alert torrent_alert;

	enum state_t state;
	enum state_t prev_state;
};

struct fastresume_rejected_alert
{
	struct torrent_alert torrent_alert;

	struct error_code error;
};

struct peer_blocked_alert
{
	struct alert alert;

	int ip;
};

struct storage_moved_alert
{
	struct torrent_alert torrent_alert;

	const char* path;
};

struct storage_moved_failed_alert
{
	struct torrent_alert torrent_alert;

	struct error_code error;
};

struct save_resume_data_alert
{
	struct torrent_alert torrent_alert;

	char* resume_data;
	int resume_data_size;
};

struct save_resume_data_failed_alert
{
	struct torrent_alert torrent_alert;

	error_code error;
};

enum stats_channel
{
	upload_payload,
	upload_protocol,
	upload_ip_protocol,
	upload_dht_protocol,
	upload_tracker_protocol,
	download_payload,
	download_protocol,
	download_ip_protocol,
	download_dht_protocol,
	download_tracker_protocol,
	stats_channel_max
};

struct stats_alert
{
	struct torrent_alert torrent_alert;

	// number of stats_channel, stats_channel_max. jnaerator 0.9.7 unable to parse transferred[stats_channel_max];
	int transferred[10];
	int interval;
};

struct dht_announce_alert
{
	struct alert alert;

	int ip;
	int port;
	const char* info_hash;
};

struct dht_get_peers_alert
{
	struct alert alert;

	const char* info_hash;
};

struct torrent_paused_alert
{
	struct torrent_alert torrent_alert;
};

struct torrent_resumed_alert
{
	struct torrent_alert torrent_alert;
};

struct torrent_finished_alert
{
	struct torrent_alert torrent_alert;
};

struct piece_finished_alert
{
	struct torrent_alert torrent_alert;

	int piece_index;
};

struct torrent_checked_alert
{
	struct torrent_alert torrent_alert;
};

struct metadata_failed_alert
{
	struct torrent_alert torrent_alert;
};

struct metadata_received_alert
{
	struct torrent_alert torrent_alert;
};

struct udp_error_alert
{
	struct alert alert;

	struct endpoint entpoint;
	struct error_code errorcode;
};

struct cache_flushed_alert
{
	struct torrent_alert torrent_alert;
};

struct file_entry
{
	const char* path;
	long long size;
	long long mtime;
	char pad_file;
	char hidden_attribute;
	char executable_attribute;
	char symlink_attribute;
	const char* symlink_path;
};

struct torrent_title
{
	const char* name;
	const char* comment;
	const char* creator;
	long long mtime;
};

#ifdef __cplusplus
extern "C"
{
#endif

// the functions whose signature ends with:
// , int first_tag, ...);
// takes a tag list. The tag list is a series
// of tag-value pairs. The tags are constants
// identifying which property the value controls.
// The type of the value varies between tags.
// The enumeration above specifies which type
// it expects. All tag lists must always be
// terminated by TAG_END.

// use SES_* tags in tag list
void* session_create(int first_tag, ...);
void session_close(void* ses);

// use TOR_* tags in tag list
void* session_add_torrent(void* ses, int first_tag, ...);

void* session_add_torrent_data(void* ses, void* resume, int resume_size, const char *target);

enum session_remove_options
{
	none = 0, delete_files = 1
};

void session_remove_torrent(void* ses, void* tor, int options);

int session_get_status(void* ses, struct session_status* s, int struct_size);

// use SET_* tags in tag list
int session_set_settings(void* ses, int first_tag, ...);
int session_get_setting(void* ses, int tag, void* value, int* value_size);

// call session_wait_alert or session_pop_alert, and then cast_to_ or session_free_alert
void* session_wait_alert(void* ses);
void* session_pop_alert(void* ses);
void session_free_alert(void* pop);

//
// torrent_*
//

int torrent_get_status(void* tor, struct torrent_status* s, int struct_size);

long torrent_get_hash(void* tor);

// use SET_* tags in tag list
int torrent_set_settings(void* tor, int first_tag, ...);
int torrent_get_setting(void* tor, int tag, void* value, int* value_size);

int torrent_save_state(void* tor);
int torrent_pause(void* tor);
int torrent_resume(void* tor);
int torrent_recheck(void* tor);
bool torrent_has_metadata(void* tor);
void* torrent_get_metadata(void* tor);
int torrent_get_metadata_size(void* tor);

// get torrent title
int torrent_info(void* tor, torrent_title* t);
// free torrent title
int torrent_info_free(void* tor, torrent_title* t);

// get files count
int torrent_files_count(void* tor);
// get file_entry struct
int torrent_files_get(void* tor, int i, struct file_entry* f);
// free file_entry struct
int torrent_files_free(void* tor, struct file_entry* f);
// rename
int torrent_files_rename(void* tor, int i, const char* f);

//
// alert_*
//

int alert_get_type(void* alert);
peer_alert * cast_to_peer_alert(void* pop);
void free_peer_alert(peer_alert * aa);
read_piece_alert * cast_to_read_piece_alert(void* pop);
void free_read_piece_alert(read_piece_alert * aa);
file_completed_alert * cast_to_file_completed_alert(void* pop);
void free_file_completed_alert(file_completed_alert * aa);
file_renamed_alert * cast_to_file_renamed_alert(void* pop);
void free_file_renamed_alert(file_renamed_alert * aa);
file_rename_failed_alert * cast_to_file_rename_failed_alert(void* pop);
void free_file_rename_failed_alert(file_rename_failed_alert * aa);
performance_alert * cast_to_performance_alert(void* pop);
void free_performance_alert(performance_alert * aa);
state_changed_alert * cast_to_state_changed_alert(void* pop);
void free_state_changed_alert(state_changed_alert * aa);
tracker_error_alert * cast_to_tracker_error_alert(void* pop);
void free_tracker_error_alert(tracker_error_alert * aa);
tracker_warning_alert * cast_to_tracker_warning_alert(void* pop);
void free_tracker_warning_alert(tracker_warning_alert * aa);
scrape_reply_alert * cast_to_scrape_reply_alert(void* pop);
void free_scrape_reply_alert(scrape_reply_alert * aa);
scrape_failed_alert * cast_to_scrape_failed_alert(void* pop);
void free_scrape_failed_alert(scrape_failed_alert * aa);
tracker_reply_alert * cast_to_tracker_reply_alert(void* pop);
void free_tracker_reply_alert(tracker_reply_alert * aa);
dht_reply_alert * cast_to_dht_reply_alert(void* pop);
void free_dht_reply_alert(dht_reply_alert * aa);
tracker_announce_alert * cast_to_tracker_announce_alert(void* pop);
void free_tracker_announce_alert(tracker_announce_alert * aa);
hash_failed_alert * cast_to_hash_failed_alert(void* pop);
void free_hash_failed_alert(hash_failed_alert * aa);
peer_ban_alert * cast_to_peer_ban_alert(void* pop);
void free_peer_ban_alert(peer_ban_alert * aa);
peer_unsnubbed_alert * cast_to_peer_unsnubbed_alert(void* pop);
void free_peer_unsnubbed_alert(peer_unsnubbed_alert * aa);
peer_snubbed_alert * cast_to_peer_snubbed_alert(void* pop);
void free_peer_snubbed_alert(peer_snubbed_alert * aa);
peer_error_alert * cast_to_peer_error_alert(void* pop);
void free_peer_error_alert(peer_error_alert * aa);
peer_connect_alert * cast_to_peer_connect_alert(void* pop);
void free_peer_connect_alert(peer_connect_alert * aa);
peer_disconnected_alert * cast_to_peer_disconnected_alert(void* pop);
void free_peer_disconnected_alert(peer_disconnected_alert * aa);
invalid_request_alert * cast_to_invalid_request_alert(void* pop);
void free_invalid_request_alert(invalid_request_alert * aa);
torrent_finished_alert * cast_to_torrent_finished_alert(void* pop);
void free_torrent_finished_alert(torrent_finished_alert * aa);
piece_finished_alert * cast_to_piece_finished_alert(void* pop);
void free_piece_finished_alert(piece_finished_alert * aa);
request_dropped_alert * cast_to_request_dropped_alert(void* pop);
void free_request_dropped_alert(request_dropped_alert * aa);
block_timeout_alert * cast_to_block_timeout_alert(void* pop);
void free_block_timeout_alert(block_timeout_alert * aa);
block_finished_alert * cast_to_block_finished_alert(void* pop);
void free_block_finished_alert(block_finished_alert * aa);
block_downloading_alert * cast_to_block_downloading_alert(void* pop);
void free_block_downloading_alert(block_downloading_alert * aa);
unwanted_block_alert * cast_to_unwanted_block_alert(void* pop);
void free_unwanted_block_alert(unwanted_block_alert * aa);
storage_moved_alert * cast_to_storage_moved_alert(void* pop);
void free_storage_moved_alert(storage_moved_alert * aa);
storage_moved_failed_alert * cast_to_storage_moved_failed_alert(void* pop);
void free_storage_moved_failed_alert(storage_moved_failed_alert * aa);
torrent_deleted_alert * cast_to_torrent_deleted_alert(void* pop);
void free_torrent_deleted_alert(torrent_deleted_alert * aa);
torrent_delete_failed_alert * cast_to_torrent_delete_failed_alert(void* pop);
void free_torrent_delete_failed_alert(torrent_delete_failed_alert * aa);
save_resume_data_alert * cast_to_save_resume_data_alert(void* pop);
void free_save_resume_data_alert(save_resume_data_alert * aa);
save_resume_data_failed_alert * cast_to_save_resume_data_failed_alert(void* pop);
void free_save_resume_data_failed_alert(save_resume_data_failed_alert * aa);
torrent_paused_alert * cast_to_torrent_paused_alert(void* pop);
void free_torrent_paused_alert(torrent_paused_alert * aa);
torrent_resumed_alert * cast_to_torrent_resumed_alert(void* pop);
void free_torrent_resumed_alert(torrent_resumed_alert * aa);
torrent_checked_alert * cast_to_torrent_checked_alert(void* pop);
void free_torrent_checked_alert(torrent_checked_alert * aa);
url_seed_alert * cast_to_url_seed_alert(void* pop);
void free_url_seed_alert(url_seed_alert * aa);
file_error_alert * cast_to_file_error_alert(void* pop);
void free_file_error_alert(file_error_alert * aa);
metadata_failed_alert * cast_to_metadata_failed_alert(void* pop);
void free_metadata_failed_alert(metadata_failed_alert * aa);
metadata_received_alert * cast_to_metadata_received_alert(void* pop);
void free_metadata_received_alert(metadata_received_alert * aa);
udp_error_alert * cast_to_udp_error_alert(void* pop);
void free_udp_error_alert(udp_error_alert * aa);
external_ip_alert * cast_to_external_ip_alert(void* pop);
void free_external_ip_alert(external_ip_alert * aa);
listen_failed_alert * cast_to_listen_failed_alert(void* pop);
void free_listen_failed_alert(listen_failed_alert * aa);
listen_succeeded_alert * cast_to_listen_succeeded_alert(void* pop);
void free_listen_succeeded_alert(listen_succeeded_alert * aa);
portmap_error_alert * cast_to_portmap_error_alert(void* pop);
void free_portmap_error_alert(portmap_error_alert * aa);
portmap_alert * cast_to_portmap_alert(void* pop);
void free_portmap_alert(portmap_alert * aa);
portmap_log_alert * cast_to_portmap_log_alert(void* pop);
void free_portmap_log_alert(portmap_log_alert * aa);
fastresume_rejected_alert * cast_to_fastresume_rejected_alert(void* pop);
void free_fastresume_rejected_alert(fastresume_rejected_alert * aa);
peer_blocked_alert * cast_to_peer_blocked_alert(void* pop);
void free_peer_blocked_alert(peer_blocked_alert * aa);
dht_announce_alert * cast_to_dht_announce_alert(void* pop);
void free_dht_announce_alert(dht_announce_alert * aa);
dht_get_peers_alert * cast_to_dht_get_peers_alert(void* pop);
void free_dht_get_peers_alert(dht_get_peers_alert * aa);
stats_alert * cast_to_stats_alert(void* pop);
void free_stats_alert(stats_alert * aa);
cache_flushed_alert * cast_to_cache_flushed_alert(void* pop);
void free_cache_flushed_alert(cache_flushed_alert * aa);

#ifdef __cplusplus
}
#endif

#endif

