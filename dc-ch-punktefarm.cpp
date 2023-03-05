#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>  

#include "strutil.h"
#include "cxxopts.hpp"
#include <curl/curl.h>

std::string username, password, cookie, rang, punkte;
std::string userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/110.0.0.0 Safari/537.36";

static size_t header_callback(char* buffer, size_t size, size_t nmemb, void* param) {
	std::string& text = *static_cast<std::string*>(param);
	text.append(static_cast<char*>(buffer), (size * nmemb));
	return size * nmemb;
}

size_t write_data(void* buffer, size_t size, size_t nmemb, void* param){
	std::string& text = *static_cast<std::string*>(param);
	text.append(static_cast<char*>(buffer), (size * nmemb));
	return size * nmemb;
}

size_t write_data_bl(void* buffer, size_t size, size_t nmemb, void* param) {
	return size * nmemb;
}

void setCookie() {
    CURL* curl;
    CURLcode res;
	std::string data;
	curl = curl_easy_init();
	std::string postData = "username=" + username + "&password=" + password + "&autologin=1";
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/ssl/certs/ca-certificates.crt");
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.detektiv-conan.ch/login.php");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_bl);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &data);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "9.9.9.9:53,149.112.112.112:53");
		curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if (CURLE_OK != res) {
			std::time_t t = std::time(nullptr);
			char buffer[100];
			std::strftime(buffer, sizeof(buffer), "[%d/%m/%y %T] ", std::localtime(&t));
			std::cerr << buffer << "(setCookie)CURL error: " << res << ", " << curl_easy_strerror(res) << std::endl;
			return;
		}

		if (strutil::contains(data, "PHPSESSID") && strutil::contains(data, "autologin")) {
			cookie.clear();
			cookie = "PHPSESSID" + strutil::split(strutil::split(data, "PHPSESSID")[1], ";")[0] + ";";
			cookie += "autologin" + strutil::split(strutil::split(data, "autologin")[1], ";")[0] + ";";
			std::time_t t = std::time(nullptr);
			char buffer[100];
			std::strftime(buffer, sizeof(buffer), "[%d/%m/%y %T] ", std::localtime(&t));
			std::cout << buffer << "Got Login Cookie" << std::endl;
			return;
		}
		std::time_t t = std::time(nullptr);
		char buffer[100];
		std::strftime(buffer, sizeof(buffer), "[%d/%m/%y %T] ", std::localtime(&t));
		std::cout << buffer << "Kein Login Cookie gefunden, wird in 30 sek nochmal getestet" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(30));
		setCookie();
	}
	return;
}

void getpnt() {
	CURL* curl;
	CURLcode res;
	std::string data;
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/ssl/certs/ca-certificates.crt");
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.detektiv-conan.ch/index.php?page=profile/profil.php");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "9.9.9.9:53,149.112.112.112:53");
		curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookie.c_str());
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if (CURLE_OK != res) {
			std::time_t t = std::time(nullptr);
			char buffer[100];
			std::strftime(buffer, sizeof(buffer), "[%d/%m/%y %T] ", std::localtime(&t));
			std::cerr << buffer << "(getpnt)CURL error: " << res << ", " << curl_easy_strerror(res) << std::endl;
		}

		if (strutil::contains(data, "Punkte: ") && strutil::contains(data, "Rang:")) {
			punkte = strutil::split(strutil::split(data, "Punkte: ")[1], "<")[0];
			rang = strutil::split(strutil::split(data, "Rang: ")[1], "<")[0];

			std::time_t t = std::time(nullptr);
			char buffer[100];
			std::strftime(buffer, sizeof(buffer), "[%d/%m/%y %T] ", std::localtime(&t));
			std::cout << buffer << "Punkte: " + punkte << ", Rang: " + rang << std::endl;
			return;
		}
		std::time_t t = std::time(nullptr);
		char buffer[100];
		std::strftime(buffer, sizeof(buffer), "[%d/%m/%y %T] ", std::localtime(&t));
		std::cout << buffer << "Keine Punkte, Rang gefunden -> Get Login Cookie in 30 Sek" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(30));
		setCookie();
		getpnt();
	}
	return;
}

int main(int argc, char** argv) {
	std::cout << "dc-ch-punktefarm ver. 1.0.0\n" << std::endl;
	cxxopts::Options options("dc-ch-punktefarm 1.0.0", "farm points.");

	options.add_options()
		("u, usr", "Username", cxxopts::value<std::string>())
		("p, pwd", "Password", cxxopts::value<std::string>())
		("v,version", "Print version")
		("h,help", "Print usage")
		;
	auto result = options.parse(argc, argv);

	if (result.count("help")) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	if (result.count("version")) {
		std::cout << "Build with: \n" + std::string(curl_version()) + " cxxopts/" << (int)cxxopts::version.major << "." << (int)cxxopts::version.minor << "." << (int)cxxopts::version.patch << "\n" << std::endl;
		return 0;
	}

	if (!result.count("usr") || !result.count("pwd")) {
		std::cout << "Error: Username and Password are required arguments!" << std::endl;
		return 0;
	}

	username = result["usr"].as<std::string>();
	password = result["pwd"].as<std::string>();

	curl_global_init(CURL_GLOBAL_DEFAULT);
	setCookie();
	while (true) {
		getpnt();
		std::this_thread::sleep_for(std::chrono::hours(2));
	}
    curl_global_cleanup();
	return 0;
}
