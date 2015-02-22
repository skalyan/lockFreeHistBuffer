/**
 * Benchmark for traditional switch-driven HTTP state machine (the code
 * is mainly borrowed from Nginx) with HTTP Hybrid State Machine.
 */
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>

#include "http.h"

#define STR(s)	{s, sizeof(s) - 1}

static struct {
	const char	*str;
	size_t 		len;
} headers[] = {
	STR("Host: github.com\r\n"),
	STR("Connection: keep-alive\r\n"),
	STR("Cache-Control: max-age=0\r\n"),
	STR("User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/535.11 (KHTML, like Gecko) Chrome/17.0.963.56 Safari/535.11\r\n"),
	STR("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"),
	STR("Accept-Encoding: gzip,deflate,sdch\r\n"),
	STR("Accept-Language: zh-CN,zh;q=0.8,en;q=0.6\r\n"),
	STR("Accept-Charset: gb18030,utf-8;q=0.7,*;q=0.3\r\n"),
	STR("If-None-Match: 7f9c6a2baf61233cedd62ffa906b604f\r\n"),
	STR("Cookie: tracker=http%3A%2F%2Fnodejs.org%2F; _gh_sess=BAh7DyIVemVkc2hhdy9tb25ncmVsMnsGOhhpc3N1ZV92aWV3X3NldHRpbmdzewgiCXNvcnQiDGNyZWF0ZWQiDmRpcmVjdGlvbiIJZGVzYyIKc3RhdGUiC2Nsb3NlZDoQX2NzcmZfdG9rZW4iMUw0eVBPdE5SVXU4eHYwZlRuZFJHY2x6QmNkYUlzclBoalhpZWR5a2NzS3M9OhBmaW5nZXJwcmludCIlYTM3YTg2ODQ0M2Q4ZWJiZDM4OGM4NThlMTc3OWMwZTM6DGNvbnRleHQiBi86D3Nlc3Npb25faWQiJWQ2ODVjZjM5YTcxZTg5NmZkYmI0NmNlMDY3NmUwMGFlIhNyeS9odHRwLXBhcnNlcnsAOhFsb2NhbGVfZ3Vlc3MiB3poIhhwaGVuZHJ5eC9zdXBlcnB1dHR5ewAiCmZsYXNoSUM6J0FjdGlvbkNvbnRyb2xsZXI6OkZsYXNoOjpGbGFzaEhhc2h7AAY6CkB1c2VkewA6CXVzZXJpA57pEQ%3D%3D--e3154a27f5cdb7f1a8b0351f997b7e3d752f4636; spy_repo=joyent%2Fhttp-parser; spy_repo_at=Sun%20Feb%2019%202012%2015%3A20%3A31%20GMT%2B0800%20(%E4%B8%AD%E5%9B%BD%E6%A0%87%E5%87%86%E6%97%B6%E9%97%B4); __utma=1.1355277945.1305645384.1329633368.1329635599.209; __utmc=1; __utmz=1.1327920052.183.88.utmcsr=nodejs.org|utmccn=(referral)|utmcmd=referral|utmcct=/\r\n"),
},

requests[] = {
	STR("GET / HTTP/1.1\r\n"),
	STR("GET ftp://mail.ru/index.html HTTP/1.1\r\n"),
	STR("POST /script1?a=44,fd=6 HTTP/1.1\r\n"),
	STR("GET /joyent/http-parser HTTP/1.1\r\n"),
	STR("PUT   http://mail.ru/index.html HTTP/1.1\r\n"),
	STR("POST /api/2/thread/404435440?1340553000964 HTTP/1.1\r\n"),
	STR("GET http://pipelined-host-C.co.uk/somepage.abc/hjkhasdfdaf23df$#ffgse4wds/fdsgsg/sfdgfg/sfdgsf0fsgfg/sfgfs/0dsdfsggsgfgsdfdsdgdfsg/345/sdfgf/4er/3453/gnnv,/,m,/5463234/567&*%&*$&3/gfg/ggdh/gdhgdhgdhg/00 HTTP/1.1\n"),
	STR("GET /pixel;r=657222568;a=p-2945K0QbJw0BA;fpan=0;fpa=P0-456992954-1322415728212;ns=0;ce=1;je=0;sr=1280x800x24;enc=n;dst=1;et=1340553300515;tzo=-240;ref=;url=http%3A%2F%2Fitman.livejournal.com%2F474249.html%3Fthread%3D5941385%23t5941385;ogl=title.%D0%9F%D0%BE%D1%87%D0%B5%D0%BC%D1%83%20%D0%BA%D0%BE%D0%BC%D0%BF%D1%8C%D1%8E%D1%82%D0%B5%D1%80%20--%20%D1%8D%D1%82%D0%BE%20%D0%BD%D0%B5%20%D0%BA%D0%BE%D0%BD%D0%B5%D1%87%D0%BD%D1%8B%D0%B9%20%D0%B0%D0%B2%D1%82%D0%BE%D0%BC%D0%B0%D1%82%3F%2Cdescription.%D0%A1%D1%82%D0%BE%D0%BB%D0%B5%D1%82%D0%B8%D1%8E%20%D0%A2%D1%8C%D1%8E%D1%80%D0%B8%D0%BD%D0%B3%D0%B0%20%D0%BF%D0%BE%D1%81%D0%B2%D1%8F%D1%89%D0%B0%D0%B5%D1%82%D1%81%D1%8F%252E%20%D0%9E%D0%BA%D0%B0%D0%B7%D1%8B%D0%B2%D0%B0%D0%B5%D1%82%D1%81%D1%8F%252C%20%D0%BE%D0%B3%D1%80%D0%BE%D0%BC%D0%BD%D0%BE%D0%B5%20%D0%BA%D0%BE%D0%BB%D0%B8%D1%87%D0%B5%D1%81%D1%82%D0%B2%D0%BE%20%D0%BB%D1%8E%D0%B4%D0%B5%D0%B9%20%D1%81%D1%87%D0%B8%D1%82%D0%B0%D0%B5%D1%82%252C%20%D1%87%2Cimage.http%3A%2F%2Fl-userpic%252Elivejournal%252Ecom%2F113387160%2F8313909 HTTP/1.1\r\n"),
};

static inline unsigned long
tv_to_ms(const struct timeval *tv)
{
	return ((unsigned long)tv->tv_sec * 1000000 + tv->tv_usec) / 1000;
}

static ngx_http_request_t r;

#define test(data, fn)							\
do {									\
	struct timeval tv0, tv1;					\
									\
	gettimeofday(&tv0, NULL);					\
									\
	for (int i = 0; i < 100 * 1000; ++i)				\
		for (int j = 0; j < sizeof(data)/sizeof(data[0]); ++j) { \
			r.state = r.lowcase_index = 0;			\
			fn(&r, (unsigned char *)data[j].str, data[j].len); \
		}							\
									\
	gettimeofday(&tv1, NULL);					\
									\
	printf("\t" #fn ":\t%lums\n", tv_to_ms(&tv1) - tv_to_ms(&tv0)); \
} while (0)

int
main()
{
	printf("Nginx HTTP parser:\n");
	test(requests, ngx_request_line);
	test(headers, ngx_header_line);
	test(headers, ngx_lw_header_line);
	test(headers, ngx_big_header_line);

	printf("\nHTTP Hybrid State Machine:\n");
	test(headers, hsm_header_line);

	printf("\nTable-driven Automaton (DPI)\n");
	test(headers, tbl_header_line);
	test(headers, tbl_big_header_line);

	printf("\nGoto-driven Automaton:\n");
	test(requests, goto_request_line);
	test(requests, goto_opt_request_line);
	test(headers, goto_header_line);
	test(headers, goto_big_header_line);

	printf("\n[req: %d %d:%d %p %p %p %p %p %p]\n",
		r.http_major, r.http_minor,
		r.state, r.header_name_start,
		r.header_start, r.request_start, r.schema_start,
		r.uri_start, r.host_start);

	return 0;
}
