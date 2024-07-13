//
// Created by ivory on 24-7-14.
//

#include "http_connection.h"

void http_connection::init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd,
    string sqlname) {
}

void http_connection::close_conn(bool real_close) {
}

void http_connection::process() {
}

bool http_connection::read_once() {
}

bool http_connection::write() {
}

void http_connection::initmysql_result(connection_pool *connPool) {
}

void http_connection::init() {
}

http_connection::HTTP_STATE http_connection::process_read() {
}

bool http_connection::process_write(HTTP_STATE ret) {
}

http_connection::HTTP_STATE http_connection::parse_request_line(char *text) {
}

http_connection::HTTP_STATE http_connection::parse_headers(char *text) {
}

http_connection::HTTP_STATE http_connection::parse_content(char *text) {
}

http_connection::HTTP_STATE http_connection::do_request() {
}

http_connection::LINE_STATUS http_connection::parse_line() {
}

void http_connection::unmap() {
}

bool http_connection::add_response(const char *format, ...) {
}

bool http_connection::add_content(const char *content) {
}

bool http_connection::add_status_line(int status, const char *title) {
}

bool http_connection::add_headers(int content_length) {
}

bool http_connection::add_content_type() {
}

bool http_connection::add_content_length(int content_length) {
}

bool http_connection::add_linger() {
}

bool http_connection::add_blank_line() {
}
