#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

class WebCrawler {
    std::mutex queue_mutex;
    std::mutex status_mutex;

    std::queue<std::string> links_queue;
    std::set<std::string> visited_links;
    std::vector<std::thread> threads;

    std::string root_link;
    size_t thread_count;
    size_t links_count = 0;

    static bool save_file_content(const std::string& link, std::string& content) {
        const std::string path = "test_data/" + link.substr(link.find_last_of('/') + 1);
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file " << path << std::endl;
            return false;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        return true;
    }

    std::vector<std::string> extract_links(const std::string& content) {
        std::vector<std::string> links;
        std::regex link_regex("<a href=\"(file://[^\"]+)\">");
        std::smatch match;

        auto it = content.cbegin();
        while (std::regex_search(it, content.cend(), match, link_regex)) {
            links.push_back(match[1].str());
            it = match.suffix().first;
        }

        return links;
    }

    static void save_page(const std::string& link, const std::string& content) {
        try {
            const std::string downloads_path = "downloads";
            std::filesystem::create_directories(downloads_path);

            std::string filename = "downloaded_" + link.substr(link.find_last_of('/') + 1);
            if (filename.empty()) {
                std::cerr << "Invalid link format: " << link << std::endl;
                return;
            }

            std::string full_path = downloads_path + "/" + filename;
            std::ofstream output_file(full_path);
            if (!output_file.is_open()) {
                std::cerr << "Failed to create file: " << full_path << std::endl;
                return;
            }
            output_file << content;
            output_file.close();

            //std::cout << "File saved: " << full_path << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error saving file: " << e.what() << std::endl;
        }
    }

    void crawler() {
        while (true) {
            std::string current_link;
            {
                std::lock_guard<std::mutex> queue_lock(queue_mutex);
                if (links_queue.empty()) { return; }
                current_link = links_queue.front();
                links_queue.pop();
            }
            {
                std::lock_guard<std::mutex> status_lock(status_mutex);
                if (visited_links.find(current_link) != visited_links.end()) { continue; }
                visited_links.insert(current_link);
                ++links_count;
            }
            if (std::string content; save_file_content(current_link, content)) {
                save_page(current_link, content);
                auto links = extract_links(content);
                {
                    std::lock_guard<std::mutex> queue_lock(queue_mutex);
                    for (const auto& link : links) {
                        if (visited_links.find(link) == visited_links.end()) {
                            links_queue.push(link);
                        }
                    }
                }
            }
        }
    }
public:
    WebCrawler(const std::string& root_link, const size_t thread_count = 1) :
        root_link(root_link), thread_count(thread_count) {
    }

    std::tuple<size_t, std::chrono::duration<double>> run() {
        const auto start = std::chrono::high_resolution_clock::now(); {
            std::lock_guard<std::mutex> queue_lock(queue_mutex);
            links_queue.push(root_link);
        }

        for (size_t i = 0; i < thread_count; ++i) {
            threads.emplace_back(&WebCrawler::crawler, this);
        }

        for (auto& thread : threads) {
            thread.join();
        }
        const auto end = std::chrono::high_resolution_clock::now();
        const auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

        return std::make_tuple(links_count, elapsed_time);
    }
};

void data_output(const std::vector<std::tuple<size_t, double, size_t>>& test_results, std::ofstream& file) {
    constexpr int width1 = 15;
    constexpr int width2 = 20;
    constexpr int width3 = 15;

    file << std::left
        << std::setw(width1) << "Thread count"
        << std::setw(width2) << "Crawling time (s)"
        << std::setw(width3) << "Links found"
        << std::endl;
    file << std::string(width1 + width2 + width3, '-') << std::endl;

    for (const auto& [thread_count, elapsed_time, links_count] : test_results) {
        file << std::left
            << std::setw(width1) << thread_count
            << std::setw(width2) << std::fixed << std::setprecision(2) << elapsed_time
            << std::setw(width3) << links_count
            << std::endl;
    }
}

#if 1
int main() {
    std::ifstream input_file("input.txt");
    std::ofstream output_file("output.txt");

    std::string root_link;
    size_t max_thread_count;

    input_file >> root_link >> max_thread_count;

    std::vector<std::tuple<size_t, double, size_t>> test_results;

    for (size_t thread_count = 1; thread_count <= max_thread_count; ++thread_count) {
        WebCrawler crawler(root_link, thread_count);

        auto [links_count, elapsed_time] = crawler.run();

        std::cout << "Thread count: " << thread_count << std::endl;
        std::cout << "Crawling completed in " << elapsed_time.count() << " seconds.\n";
        std::cout << "Total links found: " << links_count << "\n\n";

        test_results.emplace_back(thread_count, elapsed_time.count(), links_count);
    }

    data_output(test_results, output_file);

    input_file.close();
    output_file.close();
    return 0;
}
#endif

#if 0
int main() {
    const std::string root_link = "file://0.html";
    constexpr size_t thread_count = 4;

    WebCrawler crawler(root_link, thread_count);
    auto [links_count, elapsed_time] = crawler.run();

    std::cout << "Thread count: " << thread_count << std::endl;
    std::cout << "Crawling completed in " << elapsed_time.count() << " seconds.\n";
    std::cout << "Total links found: " << links_count << "\n";
}
#endif
