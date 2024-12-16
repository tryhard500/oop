#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <regex>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

std::mutex queue_mutex;   // Мьютекс для доступа к очереди
std::mutex stats_mutex;   // Мьютекс для статистики

std::queue<std::string> url_queue;          // Очередь URL-адресов
std::set<std::string> visited_urls;        // Множество посещённых страниц
size_t found_links = 0;                    // Количество найденных ссылок

// Функция для скачивания страницы
void download_page(const std::string &url, const std::string &output_path) {
    fs::copy(url.substr(7), output_path, fs::copy_options::overwrite_existing); // Локальный файл
}

// Функция для обработки страницы
void process_page(const std::string &url) {
    // Загружаем страницу
    std::string output_path = "temp_page.html";
    download_page(url, output_path);

    // Открываем файл для поиска ссылок
    std::ifstream file(output_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << output_path << std::endl;
        return;
    }

    std::string line;
    std::regex link_regex("<a href=\"(file://[^\"]+)\">");
    std::smatch match;

    while (std::getline(file, line)) {
        std::string::const_iterator search_start(line.cbegin());
        while (std::regex_search(search_start, line.cend(), match, link_regex)) {
            std::string found_url = match[1].str();
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                if (visited_urls.find(found_url) == visited_urls.end()) {
                    url_queue.push(found_url);
                    visited_urls.insert(found_url);
                }
            }
            {
                std::lock_guard<std::mutex> lock(stats_mutex);
                ++found_links;
            }
            search_start = match.suffix().first;
        }
    }
    file.close();
    fs::remove(output_path); // Удаляем временный файл
}

// Потоковая функция
void worker() {
    while (true) {
        std::string current_url;
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (url_queue.empty()) {
                break;
            }
            current_url = url_queue.front();
            url_queue.pop();
        }
        process_page(current_url);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <start_url> <num_threads>" << std::endl;
        return 1;
    }

    std::string start_url = argv[1];
    int num_threads = std::stoi(argv[2]);

    if (!fs::exists(start_url.substr(7))) {
        std::cerr << "File not found: " << start_url << std::endl;
        return 1;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        url_queue.push(start_url);
        visited_urls.insert(start_url);
    }

    // Запускаем потоки
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    // Ждём завершения всех потоков
    for (auto &t : threads) {
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time = end_time - start_time;

    // Выводим статистику
    std::cout << "Visited pages: " << visited_urls.size() << std::endl;
    std::cout << "Found links: " << found_links << std::endl;
    std::cout << "Elapsed time: " << elapsed_time.count() << " seconds" << std::endl;

    return 0;
}
