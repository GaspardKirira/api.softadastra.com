#include "products/services/ProductCache.hpp"
#include <crow.h>
#include <crow/middlewares/cors.h>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <iostream>
#include <nlohmann/json.hpp>

static std::unique_ptr<ProductCache> g_productCache;
static std::once_flag init_flag;

void registerProductRoutes(crow::App<crow::CORSHandler>& app) {
    const char* path = std::getenv("PRODUCT_JSON_PATH");
    if (!path) {
        throw std::runtime_error("❌ PRODUCT_JSON_PATH non défini !");
    }

    // ✅ Initialisation du cache dès l'enregistrement des routes
    std::call_once(init_flag, [&]() {
        g_productCache = std::make_unique<ProductCache>(path);
        g_productCache->getJsonResponse(); // charge dès le démarrage
        std::cout << "✅ [ProductRoutes] Cache produit initialisé.\n";
    });

    // 🟢 Mini documentation des routes produits
    CROW_ROUTE(app, "/api/products")
    ([] {
        nlohmann::json doc;
        doc["endpoints"] = {
            {
                {"method", "GET"},
                {"path", "/api/products"},
                {"description", "📘 Liste des routes produits (documentation rapide)"}
            },
            {
                {"method", "GET"},
                {"path", "/api/products/all"},
                {"description", "📦 Récupérer tous les produits depuis le cache"}
            },
            {
                {"method", "POST"},
                {"path", "/api/products/reload"},
                {"description", "🔁 Recharger les produits depuis le fichier source JSON"}
            },
            {
                {"method", "GET"},
                {"path", "/api/products/status"},
                {"description", "📊 État du cache (nombre de produits, taille JSON)"}
            }
        };

        crow::response res(doc.dump(2));
        res.set_header("Content-Type", "application/json");
        return res;
    });

    // 📦 Produits : JSON complet depuis le cache
    CROW_ROUTE(app, "/api/products/all")
    ([] {
        try {
            auto& cache = *g_productCache;
            auto responseStr = cache.getJsonResponse();

            crow::response res(responseStr);
            res.set_header("Content-Type", "application/json");
            return res;
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Erreur : ") + e.what());
        }
    });

    // 🔁 Rechargement du cache
    CROW_ROUTE(app, "/api/products/reload")
    .methods("POST"_method)
    ([] {
        if (!g_productCache) {
            return crow::response(500, "❌ Cache non initialisé");
        }

        try {
            g_productCache->reload();
            std::cout << "🔁 [ProductRoutes] Cache produit rechargé.\n";
            return crow::response(200, "✅ Cache rechargé avec succès");
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Erreur lors du rechargement : ") + e.what());
        }
    });

    // 📊 Statistiques du cache
    CROW_ROUTE(app, "/api/products/status")
    ([] {
        if (!g_productCache) {
            return crow::response(500, "❌ Cache non initialisé");
        }

        try {
            std::string jsonStr = g_productCache->getJsonResponse();
            nlohmann::json parsed = nlohmann::json::parse(jsonStr);
            size_t total = parsed["data"].size();

            nlohmann::json status;
            status["status"] = "🟢 OK";
            status["products"] = total;
            status["json_size"] = jsonStr.size();
            status["message"] = "Le cache est chargé et prêt.";

            crow::response res(status.dump(2));
            res.set_header("Content-Type", "application/json");
            return res;
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Erreur lors du diagnostic du cache : ") + e.what());
        }
    });
}
