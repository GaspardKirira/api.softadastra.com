#ifndef PRODUCT_CACHE_HPP
#define PRODUCT_CACHE_HPP

#include <string>
#include <mutex>

class ProductCache {
public:
    explicit ProductCache(const std::string& jsonPath);

    std::string getJsonResponse(); // JSON prêt à servir
    void reload();                 // Recharge et régénère le cache

private:
    void loadProducts();          // Recharge depuis JSON source
    void saveToCacheFile();       // Sauvegarde vers .cache
    bool loadFromCacheFile();     // Charge depuis .cache si existant

    std::string jsonFilePath;
    std::string cachedJson;
    bool isLoaded;
    std::mutex mutex;
};

#endif // PRODUCT_CACHE_HPP
