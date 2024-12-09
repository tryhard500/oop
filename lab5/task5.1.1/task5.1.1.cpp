#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <queue>
#include <regex>
#include <thread>
#include <mutex>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

// Мьютексы для синхронизации потоков
std::mutex queueMutex, setMutex, coutMutex;

// Очередь для страниц, которые нужно обработать
std::queue<std::string> urlQueue;

// Множество для отслеживания уже посещенных страниц
std::set<std::string> visitedUrls;

// Функция обработки страницы
void processPage(const std::string &url) {
    // Открытие локального файла (убираем префикс "file://")
    std::string localPath = url.substr(7);
    std::replace(localPath.begin(), localPath.end(), '/', '\\');
    std::ifstream inputFile(localPath);
    if (!inputFile.is_open()) {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cerr << "Error opening file:  " << url << std::endl;
        return;
    }

    // Считываем содержимое файла в строку
    std::string content((std::istreambuf_iterator<char>(inputFile)),
                        std::istreambuf_iterator<char>());

    // Регулярное выражение для поиска ссылок формата <a href="file://...">
    const std::regex linkRegex(R"(<a href="(file://[^]+)>)");
    std::sregex_iterator iter(content.begin(), content.end(), linkRegex);
    std::sregex_iterator end;

    // Обрабатываем все найденные ссылки
    while (iter != end) {
        std::string foundUrl = (*iter)[1].str(); // Извлекаем ссылку
        ++iter; {
            // Проверяем, была ли ссылка уже посещена
            std::lock_guard<std::mutex> lock(setMutex);
            if (visitedUrls.find(foundUrl) == visitedUrls.end()) {
                // Если ссылка новая, добавляем ее в множество и очередь
                visitedUrls.insert(foundUrl);
                std::lock_guard<std::mutex> queueLock(queueMutex);
                urlQueue.push(foundUrl);
            }
        }
    }

    // Копируем файл в локальную директорию "downloaded"
    std::string outputPath = "downloaded/" + fs::path(url.substr(7)).filename().string();
    fs::create_directories("downloaded"); // Создаем директорию, если она отсутствует
    fs::copy(url.substr(7), outputPath, fs::copy_options::overwrite_existing);
}

// Функция, выполняемая потоком
void worker() {
    while (true) {
        std::string currentUrl; {
            // Извлечение следующей задачи из очереди
            std::lock_guard<std::mutex> lock(queueMutex);
            if (urlQueue.empty()) break; // Если очередь пуста, поток завершает работу
            currentUrl = urlQueue.front();
            urlQueue.pop(); // Удаляем ссылку из очереди
        }

        // Обработка страницы
        processPage(currentUrl);
    }
}

int main() {
    // Читаем аргументы: стартовый URL и количество потоков
    std::string startUrl = "file://0.html";
    int threadCount = 4;

    // Записываем стартовую страницу в множество посещенных и очередь
    {
        std::lock_guard<std::mutex> lock(setMutex);
        visitedUrls.insert(startUrl); // Добавляем в множество посещенных
    } {
        std::lock_guard<std::mutex> lock(queueMutex);
        urlQueue.push(startUrl); // Добавляем в очередь
    }

    // Замеряем время работы программы
    auto startTime = std::chrono::high_resolution_clock::now();

    // Создаем и запускаем потоки
    std::vector<std::thread> threads;
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back(worker); // Запускаем функцию worker в каждом потоке
    }

    // Дожидаемся завершения всех потоков
    for (auto &thread: threads) {
        thread.join();
    }

    // Завершаем замер времени работы программы
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;

    // Выводим результаты работы
    std::cout << "Number of pages visited: " << visitedUrls.size() << std::endl;
    std::cout << "Total working time: " << elapsed.count() << " seconds" << std::endl;

    return 0;
}
