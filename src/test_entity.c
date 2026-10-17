#include "test_common.h"
#include "entity.h"

static EntityTable gT;

int main(void)
{
    Vec2Fx pos = { FX_TILE_CENTER(3), FX_TILE_CENTER(4) };
    EntityId a, b, c, pet;
    Entity* e;
    u32 i, minute;
    u8 need = 99;

    CHECK_EQ(sizeof(Entity), 56);

    /* spawn / get / despawn com geração */
    EntityTable_Init(&gT);
    a = EntityTable_Spawn(&gT, ENTITY_KIND_SIM, PET_SPECIES_NONE, 0, ENTITY_ID_NONE, "Ana", pos);
    b = EntityTable_Spawn(&gT, ENTITY_KIND_SIM, PET_SPECIES_NONE, 0, ENTITY_ID_NONE, "Bruno", pos);
    pet = EntityTable_Spawn(&gT, ENTITY_KIND_PET, PET_SPECIES_DOG, 0, a, "Rex", pos);
    CHECK(a != ENTITY_ID_NONE && b != a && pet != ENTITY_ID_NONE);
    CHECK_EQ(EntityTable_Count(&gT), 3);
    CHECK(EntityTable_Get(&gT, a) != NULL && EntityTable_Get(&gT, a)->kind == ENTITY_KIND_SIM);
    CHECK(EntityTable_Get(&gT, pet)->owner == a);
    CHECK(EntityTable_Validate(&gT));

    CHECK(EntityTable_Despawn(&gT, a));
    CHECK(EntityTable_Get(&gT, a) == NULL);                 /* id velho não resolve */
    CHECK(!EntityTable_Despawn(&gT, a));                    /* nem remove duas vezes */
    c = EntityTable_Spawn(&gT, ENTITY_KIND_SIM, PET_SPECIES_NONE, 0, ENTITY_ID_NONE, "Caio", pos);
    CHECK(ENTITY_ID_INDEX(c) == ENTITY_ID_INDEX(a));        /* mesmo slot reaproveitado... */
    CHECK(c != a && ENTITY_ID_GEN(c) == ENTITY_ID_GEN(a) + 1u);   /* ...com outra geração */
    CHECK(EntityTable_Get(&gT, a) == NULL && EntityTable_Get(&gT, c) != NULL);

    /* ids inválidos nunca resolvem nem derrubam */
    CHECK(EntityTable_Get(&gT, 0u) == NULL);
    CHECK(EntityTable_Get(&gT, 0xFFFFFFFFu) == NULL);
    CHECK(EntityTable_Get(&gT, ENTITY_ID_MAKE(0, 1)) == NULL);
    CHECK(EntityTable_Get(&gT, ENTITY_ID_MAKE(OTS3_MAX_ENTITIES + 1u, 1)) == NULL);
    CHECK(EntityTable_GetConst(&gT, ENTITY_ID_MAKE(65535, 65535)) == NULL);

    /* combinações inválidas de tipo/espécie */
    CHECK(EntityTable_Spawn(&gT, ENTITY_KIND_PET, PET_SPECIES_NONE, 0, 0, "x", pos) == ENTITY_ID_NONE);
    CHECK(EntityTable_Spawn(&gT, ENTITY_KIND_SIM, PET_SPECIES_CAT, 0, 0, "x", pos) == ENTITY_ID_NONE);
    CHECK(EntityTable_Spawn(&gT, ENTITY_KIND_PET, PET_SPECIES_COUNT, 0, 0, "x", pos) == ENTITY_ID_NONE);
    CHECK(EntityTable_Spawn(&gT, ENTITY_KIND_NONE, 0, 0, 0, "x", pos) == ENTITY_ID_NONE);

    /* pool cheio */
    EntityTable_Init(&gT);
    for (i = 0; i < OTS3_MAX_ENTITIES; i++)
        CHECK(EntityTable_Spawn(&gT, ENTITY_KIND_SIM, 0, 0, 0, "n", pos) != ENTITY_ID_NONE);
    CHECK(EntityTable_Spawn(&gT, ENTITY_KIND_SIM, 0, 0, 0, "n", pos) == ENTITY_ID_NONE);
    CHECK(EntityTable_Validate(&gT));

    /* a geração dá a volta sem nunca valer 0 */
    EntityTable_Init(&gT);
    gT.gen[0] = 0xFFFFu;
    a = EntityTable_Spawn(&gT, ENTITY_KIND_SIM, 0, 0, 0, "w", pos);
    CHECK_EQ(ENTITY_ID_GEN(a), 0xFFFF);
    CHECK(EntityTable_Despawn(&gT, a));
    CHECK_EQ(gT.gen[0], 1);
    CHECK(EntityTable_Validate(&gT));

    /* nomes: truncados, sempre terminados */
    EntityTable_Init(&gT);
    a = EntityTable_Spawn(&gT, ENTITY_KIND_SIM, 0, 0, 0, "NomeMuitoGrandeDemais", pos);
    e = EntityTable_Get(&gT, a);
    CHECK_EQ(strlen(e->name), OTS3_NAME_LEN - 1u);
    CHECK(EntityTable_Validate(&gT));
    memset(e->name, 'x', sizeof(e->name));
    CHECK(!EntityTable_Validate(&gT));                      /* sem NUL: rejeitado */

    /* decaimento exato, sem deriva, e só nas necessidades aplicáveis */
    EntityTable_Init(&gT);
    a = EntityTable_Spawn(&gT, ENTITY_KIND_SIM, 0, 0, 0, "Sim", pos);
    pet = EntityTable_Spawn(&gT, ENTITY_KIND_PET, PET_SPECIES_CAT, 0, a, "Mia", pos);
    {
        Entity* s = EntityTable_Get(&gT, a);
        Entity* p = EntityTable_Get(&gT, pet);
        CHECK(Entity_NeedApplies(s, NEED_BLADDER) && !Entity_NeedApplies(s, NEED_AFFECTION));
        CHECK(Entity_NeedApplies(p, NEED_AFFECTION) && !Entity_NeedApplies(p, NEED_BLADDER));
        CHECK_EQ(p->need[NEED_BLADDER], NEED_MAX);          /* inaplicável fica cheio */
        CHECK_EQ(s->need[NEED_AFFECTION], NEED_MAX);
        for (minute = 0; minute < 600; minute++) {
            Entity_DecayNeedsMinute(s);
            Entity_DecayNeedsMinute(p);
        }
        CHECK_EQ(s->need[NEED_HUNGER], 850 - (40 * 600) / 60);     /* 450 exato */
        CHECK_EQ(p->need[NEED_HUNGER], 850 - (36 * 600) / 60);     /* 490 exato */
        CHECK_EQ(p->need[NEED_AFFECTION], 850 - (22 * 600) / 60);  /* 630 exato */
        CHECK_EQ(p->need[NEED_BLADDER], NEED_MAX);                 /* continua travada */
        for (minute = 0; minute < 5000; minute++) Entity_DecayNeedsMinute(s);
        CHECK_EQ(s->need[NEED_HUNGER], 0);                          /* satura em 0, não negativa */
        CHECK(EntityTable_Validate(&gT));
    }

    /* AddNeed satura; MostUrgentNeed considera só o aplicável e desempata pelo menor id */
    {
        Entity* p = EntityTable_Get(&gT, pet);
        Entity_AddNeed(p, NEED_HUNGER, 100000);  CHECK_EQ(p->need[NEED_HUNGER], NEED_MAX);
        Entity_AddNeed(p, NEED_HUNGER, -100000); CHECK_EQ(p->need[NEED_HUNGER], 0);
        Entity_AddNeed(p, 200, 5);               /* id inválido: ignorado */
        p->need[NEED_ENERGY] = 0; p->need[NEED_AFFECTION] = 100;
        CHECK(Entity_MostUrgentNeed(p, 350, &need) && need == NEED_HUNGER);   /* empate 0/0: menor id */
        p->need[NEED_HUNGER] = 900; p->need[NEED_ENERGY] = 900;
        CHECK(Entity_MostUrgentNeed(p, 350, &need) && need == NEED_AFFECTION);
        p->need[NEED_AFFECTION] = 900;
        CHECK(!Entity_MostUrgentNeed(p, 350, &need));
        p->need[NEED_BLADDER] = 0;                                              /* não se aplica a pets */
        CHECK(!Entity_MostUrgentNeed(p, 350, &need));
    }

    /* validação detecta corrupção */
    {
        Entity* s = EntityTable_Get(&gT, a);
        s->need[NEED_FUN] = 1001;            CHECK(!EntityTable_Validate(&gT)); s->need[NEED_FUN] = 500;
        s->need_rem[NEED_FUN] = 60;          CHECK(!EntityTable_Validate(&gT)); s->need_rem[NEED_FUN] = 0;
        s->kind = 9;                         CHECK(!EntityTable_Validate(&gT)); s->kind = ENTITY_KIND_SIM;
        s->species = PET_SPECIES_DOG;        CHECK(!EntityTable_Validate(&gT)); s->species = 0;
        s->flags = 0x80;                     CHECK(!EntityTable_Validate(&gT)); s->flags = ENTITY_FLAG_ACTIVE;
        gT.gen[ENTITY_ID_INDEX(a) - 1u] = 7; CHECK(!EntityTable_Validate(&gT));   /* id x geração */
        gT.gen[ENTITY_ID_INDEX(a) - 1u] = 1; CHECK(EntityTable_Validate(&gT));
    }
    return Test_Finish("entity");
}
