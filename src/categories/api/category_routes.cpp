#include "categories/services/CategoryCache.hpp"
#include <crow.h>
#include <crow/middlewares/cors.h>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <iostream>
#include <nlohmann/json.hpp>

static std::unique_ptr<CategoryCache> g_leafCache;
static std::unique_ptr<CategoryCache> g_topCache;
static std::unique_ptr<CategoryCache> g_allCache;

static std::once_flag flagLeaf, flagTop, flagAll;

void registerCategoryRoutes(crow::App<crow::CORSHandler>& app) {
    const char* pathLeaf = std::getenv("CATEGORY_JSON_PATH");
    const char* pathTop = std::getenv("CATEGORY_JSON_PATH_TOP_LEVEL");
    const char* pathAll = std::getenv("CATEGORY_JSON_PATH_ALL");

    if (!pathLeaf || !pathTop || !pathAll) {
        throw std::runtime_error("❌ Un des chemins JSON de catégorie n'est pas défini");
    }

    std::call_once(flagLeaf, [&] {
        g_leafCache = std::make_unique<CategoryCache>(pathLeaf);
        g_leafCache->getLeafCategoriesJson();
        std::cout << "✅ [CategoryCache] Feuilles initialisées\n";
    });

    std::call_once(flagTop, [&] {
        g_topCache = std::make_unique<CategoryCache>(pathTop);
        g_topCache->getTopLevelCategoriesJson();
        std::cout << "✅ [CategoryCache] Top-level initialisées\n";
    });

    std::call_once(flagAll, [&] {
        g_allCache = std::make_unique<CategoryCache>(pathAll);
        g_allCache->getAllCategoriesJson();
        std::cout << "✅ [CategoryCache] Toutes les catégories initialisées\n";
    });

    CROW_ROUTE(app, "/api/categories/leaf")([] {
        try {
            crow::response res(g_leafCache->getLeafCategoriesJson());
            res.set_header("Content-Type", "application/json");
            res.set_header("Cache-Control", "public, max-age=60");
            return res;
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Erreur serveur : ") + e.what());
        }
    });

    CROW_ROUTE(app, "/api/categories/top")([] {
        try {
            crow::response res(g_topCache->getTopLevelCategoriesJson());
            res.set_header("Content-Type", "application/json");
            res.set_header("Cache-Control", "public, max-age=60");
            return res;
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Erreur serveur : ") + e.what());
        }
    });

    CROW_ROUTE(app, "/api/categories/all")([] {
        try {
            crow::response res(g_allCache->getAllCategoriesJson());
            res.set_header("Content-Type", "application/json");
            res.set_header("Cache-Control", "public, max-age=60");
            return res;
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Erreur serveur : ") + e.what());
        }
    });

    CROW_ROUTE(app, "/api/categories/status")([] {
        try {
            if (!g_leafCache || !g_topCache || !g_allCache) {
                return crow::response(500, "❌ Caches non initialisés");
            }

            nlohmann::json status;
            status["status"] = "🟢 OK";
            status["leaf_count"] = g_leafCache->getLeafCount();
            status["top_level_count"] = g_topCache->getTopLevelCount();
            status["all_count"] = g_allCache->getAllCount();
            status["message"] = "Les caches de catégories sont chargés et disponibles.";

            crow::response res(status.dump(2));
            res.set_header("Content-Type", "application/json");
            res.set_header("Cache-Control", "no-store");
            return res;
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Erreur lors du status des catégories : ") + e.what());
        }
    });

    CROW_ROUTE(app, "/api/categories/reload").methods("POST"_method)([] {
        try {
            if (g_leafCache) g_leafCache->reload();
            if (g_topCache) g_topCache->reload();
            if (g_allCache) g_allCache->reload();
            std::cout << "♻️ [CategoryCache] Tous les caches rechargés\n";
            return crow::response(200, "✅ Caches rechargés avec succès");
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Erreur lors du rechargement : ") + e.what());
        }
    });
}