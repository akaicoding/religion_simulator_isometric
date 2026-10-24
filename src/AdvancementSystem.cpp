#include "AdvancementSystem.hpp"
#include <algorithm>

namespace ReligionSim {

AdvancementSystem::AdvancementSystem() {
    m_doctrineCatalog = {
        {DoctrineType::AnimistSpirits, "Animist Communion", "Spirits animate the rivers, winds and stones.",
         HistoricalEra::PrimordialAnimism, 0, 1.0f, 1.2f, 1.0f},

        {DoctrineType::SacredSacrifice, "Totemic Ancestor Rituals", "Clan totems and relics appease ancestral spirits.",
         HistoricalEra::TotemismAncestorWorship, 50, 1.15f, 0.9f, 1.1f},

        {DoctrineType::AsceticMonasticism, "Shamanic Megaliths & Harvest", "Trance states, soil fertility rituals, and stone megaliths.",
         HistoricalEra::StructuralShamanism, 100, 1.25f, 1.1f, 1.15f},

        {DoctrineType::PantheonPolytheism, "Territorial Pantheon", "Ziggurats, city-state pantheons, and temple tithing.",
         HistoricalEra::TerritorialPolytheism, 200, 1.35f, 1.2f, 1.1f},

        {DoctrineType::ImperialDivineRight, "Sacred Sacerdotal Hierarchy", "King-priests and codified ritual law govern the state.",
         HistoricalEra::TerritorialPolytheism, 350, 1.45f, 0.8f, 1.3f},

        {DoctrineType::WrittenCanon, "Universal Holy Scripture", "Single codified truth in scripture for personal salvation.",
         HistoricalEra::UniversalReligions, 550, 1.6f, 0.7f, 1.4f},

        {DoctrineType::ProselytizingZeal, "Universal Mission & Pilgrimage", "Monastic orders and pilgrims evangelize continental realms.",
         HistoricalEra::UniversalReligions, 800, 1.85f, 0.5f, 1.3f},

        {DoctrineType::SyncreticOpenness, "Ecumenical Fusion", "Dialogue and assimilation of syncretic pantheons.",
         HistoricalEra::UniversalReligions, 1050, 1.4f, 1.8f, 1.0f},

        {DoctrineType::PrintingEvangelism, "Print-Era Dissemination", "Movable type multiplies tracts, igniting reformations.",
         HistoricalEra::RationalizationSecularization, 1400, 2.2f, 0.7f, 0.9f},

        {DoctrineType::SecularHumanism, "Rationality & Secular Morality", "Scientific inquiry and ethical conscience in secular nation-states.",
         HistoricalEra::RationalizationSecularization, 1800, 1.7f, 1.6f, 1.2f},

        {DoctrineType::ScholasticTheology, "Post-Secular Micro-Cults", "Customized cyber-spirituality, wellness cults, and viral networks.",
         HistoricalEra::PostSecular, 2400, 2.0f, 1.7f, 0.9f},

        {DoctrineType::CosmicTranscendence, "Post-Biological Singularity", "Quantum computing consciousness, mind uploads, and cyber-faith.",
         HistoricalEra::TranshumanistPostBiological, 3200, 2.6f, 1.5f, 1.5f}
    };
}

const EraConfig& AdvancementSystem::getEraConfig(HistoricalEra era) {
    auto idx = static_cast<std::size_t>(era);
    if (idx >= static_cast<std::size_t>(HistoricalEra::Count)) {
        idx = 0;
    }
    return HISTORICAL_ERAS_TABLE[idx];
}

std::string AdvancementSystem::getEraName(HistoricalEra era) {
    return std::string(getEraConfig(era).name);
}

std::string AdvancementSystem::getEraDescription(HistoricalEra era) {
    const auto& cfg = getEraConfig(era);
    return std::string(cfg.faith_concept) + " | Threat: " + std::string(cfg.primary_threat);
}

HistoricalEra AdvancementSystem::getEraForYear(int year) {
    if (year < -3000) return HistoricalEra::PrimordialAnimism;             // Era 1: -4000 to -3000
    if (year < -2000) return HistoricalEra::TotemismAncestorWorship;        // Era 2: -3000 to -2000
    if (year < -1000) return HistoricalEra::StructuralShamanism;           // Era 3: -2000 to -1000
    if (year < 200)   return HistoricalEra::TerritorialPolytheism;         // Era 4: -1000 to 200
    if (year < 1500)  return HistoricalEra::UniversalReligions;            // Era 5: 200 to 1500
    if (year < 1980)  return HistoricalEra::RationalizationSecularization; // Era 6: 1500 to 1980
    if (year < 2060)  return HistoricalEra::PostSecular;                   // Era 7: 1980 to 2060
    return HistoricalEra::TranshumanistPostBiological;                      // Era 8: 2060+
}

const AdvancementDoc* AdvancementSystem::getDocInfo(DoctrineType doc) const {
    for (const auto& item : m_doctrineCatalog) {
        if (item.type == doc) return &item;
    }
    return nullptr;
}

void AdvancementSystem::updateAdvancements(ReligionRegistry& registry, int currentYear,
                                          std::vector<GameEvent>& outEvents) {
    EraType worldEra = getEraForYear(currentYear);

    for (auto& r : registry.getAllReligions()) {
        if (r.isExtinct) continue;

        // Theology score gains from followers, holy sites, and scholasticism
        float pointGain = (r.totalFollowers * 0.0005f) +
                          (r.holySitesCount * 4.0f) +
                          (r.scholasticism * 10.0f) + 1.0f;
        r.theologyPoints += static_cast<int>(pointGain);

        // Check if religion transitions to new era
        if (static_cast<int>(worldEra) > static_cast<int>(r.currentEra)) {
            r.currentEra = worldEra;
            r.advancementLevel = static_cast<int>(worldEra) + 1;

            GameEvent ev;
            ev.year = currentYear;
            ev.category = EventCategory::EraAdvancement;
            ev.religionAId = r.id;
            ev.badgeColor = r.color;
            ev.title = r.name + " Enters " + getEraName(worldEra);
            ev.description = "The followers of " + r.name + " have crossed into a new epoch: " + getEraDescription(worldEra);
            outEvents.push_back(ev);
        }

        // Try unlocking doctrines matching this era
        for (const auto& doc : m_doctrineCatalog) {
            if (static_cast<int>(doc.era) <= static_cast<int>(r.currentEra)) {
                if (!r.hasDoctrine(doc.type) && r.theologyPoints >= doc.theologyCost) {
                    r.addDoctrine(doc.type);
                    r.zeal = std::clamp(r.zeal * doc.spreadModifier, 0.1f, 1.0f);
                    r.syncretismTolerance = std::clamp(r.syncretismTolerance * doc.syncretismModifier, 0.05f, 1.0f);
                    r.cohesion = std::clamp(r.cohesion * doc.cohesionModifier, 0.1f, 1.0f);
                    r.scholasticism = std::min(1.0f, r.scholasticism + 0.1f);
                }
            }
        }
    }
}

} // namespace ReligionSim
