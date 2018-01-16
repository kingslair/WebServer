#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "http_parser.h"
#include "http_helper_functions.h"
#include "helper_functions.h"
#include "http_DELETE_request_handler.h"

#include <glib.h>

GHashTable *config_hash = NULL;
GHashTable *redirect_hash = NULL;

/************************************************************************************************
 * Function : init_suite_1
 *
 * Arguments : void
 *
 * Description : initialize the test suite
 *
 * Return : int
 *
 * Author : Rajat Bajpai
 *
 * **********************************************************************************************/
int init_suite_1(void) {
	config_hash = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(config_hash, g_strdup("BASE_DIR"), g_strdup("../../../www"));
	g_hash_table_insert(config_hash, g_strdup("LOG_PATH"), g_strdup("../../../log"));
	g_hash_table_insert(config_hash, g_strdup("TRASH_PATH"), g_strdup("../../../trash"));
	g_hash_table_insert(config_hash, g_strdup("CONFIG_PATH"), g_strdup("../../../config"));


	redirect_hash = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(redirect_hash, g_strdup("../../../www/old_page.html"), g_strdup("html_page/secpage.html"));

	if (g_hash_table_size(config_hash) < 1 || g_hash_table_size(redirect_hash) < 1) {
		return -1;
	}

	return 0;
}

/************************************************************************************************
 * Function : clean_suite_1
 *
 * Arguments : void
 *
 * Description : Cleaning the test suite resources
 *
 * Return : int
 *
 * Author : Rajat Bajpai
 *
 * **********************************************************************************************/
int clean_suite_1(void) {
	g_hash_table_foreach(config_hash, (GHFunc)free_hash, NULL);
	g_hash_table_foreach(redirect_hash, (GHFunc)free_hash, NULL);

	g_hash_table_destroy(config_hash);
	g_hash_table_destroy(redirect_hash);
	return 0;
}

/************************************************************************************************
 * Function : test_http_parser_header
 *
 * Arguments : void
 *
 * Description : test the HTTP header parser method
 *
 * Return : void
 *
 * Author : Rajat Bajpai
 *
 * **********************************************************************************************/
void test_http_parser_header(void) {
	fd_set master;
	FD_ZERO(&master);
	FD_SET(0, &master);
	GHashTable *request_hash = http_parse_request_header("GET /hello.html HTTP/1.1\r\nKey1: value1\r\nkey2: value2\r\n\r\n", 0, &master, &config_hash, "10.10.10.10");
	CU_ASSERT_EQUAL(g_hash_table_size(request_hash), 4);
	CU_ASSERT_NOT_EQUAL(g_hash_table_size(request_hash), 5);
}

/************************************************************************************************
 * Function : test_http_parse_header_fields
 *
 * Arguments : void
 *
 * Description : test the HTTP parse header fields
 *
 * Return : void
 *
 * Author : Rajat Bajpai
 *
 * **********************************************************************************************/
void test_http_parse_header_fields(void) {
	fd_set master;
	FD_ZERO(&master);
	FD_SET(0, &master);
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	char **splt_req_line = g_strsplit("GET /hello.html HTTP/1.1\r\nkey1: value\r\nkey2: value\r\n\r\n", "\r\n", -1);
	int ret_val = http_parse_header_fields(splt_req_line, g_strv_length(splt_req_line), &hash, 0, &master, &config_hash, "10.10.10.10");

	CU_ASSERT_EQUAL(ret_val, 0);
	CU_ASSERT_NOT_EQUAL(ret_val, -1);
	CU_ASSERT_EQUAL(g_hash_table_size(hash), 2);
	CU_ASSERT_NOT_EQUAL(g_hash_table_size(hash), 3);

	g_strfreev(splt_req_line);
	g_hash_table_foreach(hash, (GHFunc)free_hash, NULL);
	g_hash_table_destroy(hash);

	FD_ZERO(&master);
	FD_SET(0, &master);
	hash = g_hash_table_new(g_str_hash, g_str_equal);
	splt_req_line = g_strsplit("GET /hello.html HTTP/1.1\r\nkey1 : value\r\nkey2: value2\r\n\r\n", "\r\n", -1);
	ret_val = http_parse_header_fields(splt_req_line, g_strv_length(splt_req_line), &hash, 0, &master, &config_hash, "10.10.10.10");

	CU_ASSERT_EQUAL(ret_val, -1);
	CU_ASSERT_NOT_EQUAL(ret_val, 0);
	CU_ASSERT_EQUAL(g_hash_table_size(hash), 0);
	CU_ASSERT_NOT_EQUAL(g_hash_table_size(hash), 2);

	g_strfreev(splt_req_line);
	g_hash_table_foreach(hash, (GHFunc)free_hash, NULL);
	g_hash_table_destroy(hash);
}

/************************************************************************************************
 * Function :  test_http_parse_status_line
 *
 * Arguments : void
 *
 * Description : test the HTTP parse status line.
 *
 * Return : void
 *
 * Author : Rajat Bajpai
 *
 * **********************************************************************************************/
void test_http_parse_status_line(void) {
	fd_set master;
	FD_ZERO(&master);
	FD_SET(0, &master);
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	int ret_val = http_parse_status_line("GET /hello.html HTTP/1.1", &hash, 0, &master, &config_hash, "10.10.10.10");

	CU_ASSERT_EQUAL(ret_val, 0);
	CU_ASSERT_NOT_EQUAL(ret_val, -1);
	CU_ASSERT_EQUAL(g_hash_table_size(hash), 2);
	CU_ASSERT_NOT_EQUAL(g_hash_table_size(hash), 3);

	g_hash_table_foreach(hash, (GHFunc)free_hash, NULL);
	g_hash_table_destroy(hash);
}

/************************************************************************************************
 * Function : test_http_send_response
 *
 * Arguments : void
 *
 * Description : test the HTTP send response function.
 *
 * Return : void
 *
 * Author : Rajat Bajpai
 *
 * **********************************************************************************************/
void test_http_send_response(void) {
	int ret_val = send_http_response(2, "HEADERS", "not_exists");

	CU_ASSERT_EQUAL(ret_val, 1);
	CU_ASSERT_NOT_EQUAL(ret_val, 0);

	ret_val = send_http_response(2, "HEADERS", NULL);

	CU_ASSERT_EQUAL(ret_val, 0);
	CU_ASSERT_NOT_EQUAL(ret_val, 1);
	
}

/************************************************************************************************
 * Function : test_page_access_rights
 *
 * Arguments : void
 *
 * Description : test page access rights
 *
 * Return : void
 *
 * Author : Rajat Bajpai
 *
 * **********************************************************************************************/
void test_page_access_rights(void) {
	FILE *fp = fopen("test.txt", "w");
	int ret_val = http_page_access_rights("test.txt");

	CU_ASSERT_EQUAL(ret_val, 1);
	CU_ASSERT_NOT_EQUAL(ret_val, 0);
	fclose(fp);
	remove("test.txt");
}

/************************************************************************************************
 * Function : test_valid_path_file
 *
 * Arguments : void
 *
 * Description : test the get_valid_file_path function
 *
 * Return : void
 *
 * Author : Rajat Bajpai
 *
 * **********************************************************************************************/
void test_valid_path_file(void) {
	fd_set master;
	FD_ZERO(&master);
	FD_SET(0, &master);
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(hash, g_strdup("Host"), g_strdup("http://localhost"));
	g_hash_table_insert(hash, g_strdup("method"), g_strdup("TEST"));

	char *filename = get_valid_file_path("/hello.html", 0, &master, "10.10.10.10", &config_hash, &redirect_hash, &hash);

	CU_ASSERT_STRING_EQUAL(filename, "../../../www/hello.html");
	CU_ASSERT_NOT_EQUAL(strlen(filename), 0);

	g_free(filename);
	g_hash_table_foreach(hash, (GHFunc)free_hash, NULL);
	g_hash_table_destroy(hash);
}

/************************************************************************************************
 * Function :  test_get_datestring
 *
 * Arguments :void
 *
 * Description : test the get datestring
 *
 * Return : void
 *
 * Author : Rajat Bajpai
 *
 * **********************************************************************************************/
void test_get_datestring(void) {
	char *p_datestring = get_date_time();

	CU_ASSERT_PTR_NOT_NULL(p_datestring);
	g_free(p_datestring);
}

/************************************************************************************************
 * Function :  test_get_lmd
 *
 * Arguments : void
 *
 * Description : test the last modified date
 *
 * Return : void
 *
 * Author : Rajat Bajpai
 *
 * **********************************************************************************************/
void test_get_lmd() {
	FILE *p_fp = fopen("test.txt", "w");
	char *p_lmd = get_last_modified_date("test.txt");

	CU_ASSERT_PTR_NOT_NULL(p_lmd);
	g_free(p_lmd);
	
	p_lmd = get_last_modified_date("not_exists");
	CU_ASSERT_PTR_NULL(p_lmd);
	fclose(p_fp);
	remove("test.txt");
}

/************************************************************************************************
 * Function : test_get_file_size
 *
 * Arguments : void
 *
 * Description : test the get file size functions
 * 
 * Return : void
 *
 * Author : Abhijeet Das
 *
 * **********************************************************************************************/
void test_get_file_size(void) {
	FILE *p_fp = fopen("test.txt", "w");
	int file_size = get_file_size("test.txt");

	CU_ASSERT_EQUAL(file_size, 0);

	fprintf(p_fp, "Hello");
	fflush(p_fp);
	file_size = get_file_size("test.txt");
	CU_ASSERT_EQUAL(file_size, 5);

	fclose(p_fp);
	remove("test.txt");
}

/************************************************************************************************
 * Function : test_str_to_int
 *
 * Arguments : void
 *
 * Description : tes the string to int function
 * 
 * Return :void
 *
 * Author : Abhijeet Das
 *
 * **********************************************************************************************/
void test_str_to_int(void) {
	int number = str_to_int("hello");

	CU_ASSERT_EQUAL(number, -1);

	number = str_to_int("500");
	CU_ASSERT_EQUAL(number, 500);
}

/************************************************************************************************
 * Function :  test_get_content_type
 *
 * Arguments : void
 *
 * Description : test the get content type of file
 *
 * Return : void
 *
 * Author : Rajat Bajpai
 *
 * **********************************************************************************************/
void test_get_content_type(void) {
	char *content_type = get_content_type("hello.html");
	CU_ASSERT_STRING_EQUAL(content_type, "text/html");
	g_free(content_type);

	content_type = get_content_type("test.pdf");
	CU_ASSERT_STRING_EQUAL(content_type, "application/pdf");
	g_free(content_type);

	content_type = get_content_type("test.unkown");
	CU_ASSERT_STRING_EQUAL(content_type, "text/plain");
	g_free(content_type);

}

/************************************************************************************************
 * Function : test_is_directory
 *
 * Arguments : void
 *
 * Description : test the is directory function.
 *
 * Return : void
 *
 * Author : Rajat Bajpai
 *
 * **********************************************************************************************/
void test_is_directory(void) {
	int ret_val = is_directory("test.txt");
	CU_ASSERT_EQUAL(ret_val, 0);

	FILE *p_fp = fopen("test.txt", "w");
	ret_val = is_directory("test.txt");
	CU_ASSERT_EQUAL(ret_val, 0);

	fclose(p_fp);
}

/************************************************************************************************
 * Function : test_http_DELETE_request
 *
 * Arguments : void
 *
 * Description : test the Delete function.
 *
 * Return : void
 *
 * Author : Abhijeet Das
 *
 * **********************************************************************************************/
void test_http_DELETE_request(void) {
	fd_set master;
	FD_ZERO(&master);
	FD_SET(0, &master);
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(hash, g_strdup("url"), g_strdup("/test.html"));
	int ret_val = handle_DELETE_request(0, &master, &hash, "10.10.10.10", &config_hash, &redirect_hash);
	
	CU_ASSERT_EQUAL(ret_val, 0);

}


/*********************** Main Test program starts here *****************************************/
int main() {
	CU_pSuite parse_suite = NULL;
	CU_pSuite helper_suite = NULL;
	CU_pSuite delete_suite = NULL;

	if (CUE_SUCCESS != CU_initialize_registry()) {
		return CU_get_error();
	}

	parse_suite = CU_add_suite("Suite_1", init_suite_1, clean_suite_1);
	if (NULL == parse_suite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	helper_suite = CU_add_suite("Suite_2", init_suite_1, clean_suite_1);
	if (NULL == helper_suite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	delete_suite = CU_add_suite("Suite_3", init_suite_1, clean_suite_1);
	if (NULL == delete_suite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	if ((NULL == CU_add_test(parse_suite, "Test HTTP parser", test_http_parser_header)) 
		||  (NULL == CU_add_test(parse_suite, "Test HTTP Parse Header Fields", test_http_parse_header_fields))
		||  (NULL == CU_add_test(parse_suite, "Test HTTP Parse Status Line", test_http_parse_status_line))) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	if ((NULL == CU_add_test(helper_suite, "Test send HTTP response", test_http_send_response)) 
		|| (NULL == CU_add_test(helper_suite, "Test http page access rights", test_page_access_rights)) 
		/*|| (NULL == CU_add_test(helper_suite, "Test get valid file path", test_valid_path_file))*/
		|| (NULL == CU_add_test(helper_suite, "Test datestring", test_get_datestring))
		|| (NULL == CU_add_test(helper_suite, "Test last modified path", test_get_lmd))
		|| (NULL == CU_add_test(helper_suite, "Test get file size", test_get_file_size))
		|| (NULL == CU_add_test(helper_suite, "Test string to int", test_str_to_int))
		|| (NULL == CU_add_test(helper_suite, "Test content type", test_get_content_type))
		|| (NULL == CU_add_test(helper_suite, "Test is directory function", test_is_directory))) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	if ((NULL == CU_add_test(delete_suite, "Test Delete request", test_http_DELETE_request))) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}
