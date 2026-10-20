#include "test_common.h"
#include "lifecycle.h"

static Lifecycle L;
static EntityTable ents;
static LifeEventQueue q;
static LifeContext ctx;

static void Reset(u32 seed, u32 today)
{
    Vec2Fx pos = { FX_TILE_CENTER(5), FX_TILE_CENTER(5) };
    Lifecycle_Init(&L, seed, today);
    EntityTable_Init(&ents);
    LifeEvents_Init(&q);
    ctx.ents = &ents; ctx.events = &q; ctx.hooks = NULL; ctx.spawn_pos = pos;
}

static LifeResult AddR(PersonId* out, const char* name, u32 age, u8 gender, u8 hh, u8 carry,
                       u8 wants, PersonId p0, PersonId p1, u32 today)
{
    LifePersonInit in;
    PersonId id = 0;
    LifeResult r;
    memset(&in, 0, sizeof(in));
    in.name = name; in.age_days = age; in.gender = gender; in.household = hh;
    in.can_carry = carry; in.wants_entity = wants; in.parent[0] = p0; in.parent[1] = p1;
    r = Life_AddPerson(&L, &in, today, &id);
    if (out) *out = id;
    return r;
}

static PersonId Add(const char* name, u32 age, u8 gender, u8 hh, u8 carry, u8 wants, PersonId p0, PersonId p1, u32 today)
{
    PersonId id = 0;
    CHECK_EQ(AddR(&id, name, age, gender, hh, carry, wants, p0, p1, today), LIFE_OK);
    return id;
}

static u32 Drain(u32 counts[16])
{
    LifeEvent e;
    u32 n = 0;
    if (counts) memset(counts, 0, 16 * sizeof(u32));
    while (LifeEvents_Pop(&q, &e)) { if (counts && e.type < 16) counts[e.type]++; n++; }
    return n;
}

static u32 Fnv(const u8* b, u32 n)
{
    u32 h = 2166136261u, i;
    for (i = 0; i < n; i++) { h ^= b[i]; h *= 16777619u; }
    return h;
}

static void RunDays(u32* today, u32 n)
{
    u32 i;
    for (i = 0; i < n; i++) { (*today)++; Life_Update(&L, &ctx, *today); }
}

/* ---------------------------------------------------------------- hooks -- */
static u32 gSpawnCalls, gDespawnCalls;
static EntityId HookSpawn(void* user, u8 household, const char* name, Vec2Fx pos)
{
    (void)user;
    gSpawnCalls++;
    return EntityTable_Spawn(&ents, ENTITY_KIND_SIM, PET_SPECIES_NONE, household, ENTITY_ID_NONE, name, pos);
}
static void HookDespawn(void* user, EntityId id)
{
    (void)user;
    gDespawnCalls++;
    (void)EntityTable_Despawn(&ents, id);
}

/* ============================================================== testes == */

static void TestStages(void)
{
    u32 span, st, age;
    static const u32 normal_starts[LIFE_STAGE_COUNT] = { 0, 3, 10, 24, 38, 66, 94 };
    u32 prev;

    for (st = 0; st < LIFE_STAGE_COUNT; st++)
        CHECK_EQ(Life_StageStartDays(LIFE_SPAN_NORMAL, (u8)st), normal_starts[st]);
    for (span = 0; span < LIFE_SPAN_COUNT; span++) {
        for (st = 0; st < LIFE_STAGE_COUNT; st++) {
            u32 start = Life_StageStartDays((u8)span, (u8)st);
            CHECK_EQ(Life_StageOfAge((u8)span, start), st);
            if (st > 0) CHECK_EQ(Life_StageOfAge((u8)span, start - 1u), st - 1u);
        }
        CHECK_EQ(Life_StageOfAge((u8)span, 1000000u), LIFE_STAGE_ELDER);
        /* chance de morte: 0 na graça, monótona, 1000 no limite duro */
        {
            u32 es = Life_StageStartDays((u8)span, LIFE_STAGE_ELDER);
            prev = 0;
            CHECK_EQ(Life_DeathChancePermille((u8)span, es), 0);
            for (age = es; age < es + 100u; age++) {
                u32 c = Life_DeathChancePermille((u8)span, age);
                CHECK(c >= prev && c <= 1000u);
                prev = c;
            }
            CHECK_EQ(prev, 1000);
            CHECK(Life_DeathChancePermille((u8)span, es + 100u) == 1000u);
        }
    }
    CHECK_EQ(Life_DeathChancePermille(LIFE_SPAN_NORMAL, 94 + 7), 0);
    CHECK_EQ(Life_DeathChancePermille(LIFE_SPAN_NORMAL, 94 + 8), 47);
    CHECK_EQ(Life_DeathChancePermille(LIFE_SPAN_NORMAL, 94 + 28), 1000);
    CHECK_EQ(Life_StageOfAge(200, 5), Life_StageOfAge(LIFE_SPAN_NORMAL, 5));      /* span inválido => normal */
    CHECK(Life_StageCaps(LIFE_STAGE_BABY) == 0 && !(Life_StageCaps(LIFE_STAGE_CHILD) & LIFE_CAP_PARTNER));
    CHECK(Life_StageCaps(LIFE_STAGE_ADULT) & LIFE_CAP_WORK);
}

static void TestAging(void)
{
    u32 today = 100, d, counts[16], stages_seen = 0;
    PersonId a;
    Person* p;
    LifeEvent e;

    Reset(7, today);
    Life_SetSettings(&L, LIFE_SET_AGING);                       /* sem morte natural */
    a = Add("Bebe", 0, LIFE_GENDER_FEMALE, 0, 1, 1, 0, 0, today);
    CHECK_EQ(Life_MaterializeBodies(&L, &ctx, today), 1);
    p = Life_Get(&L, a);
    CHECK(EntityTable_Get(&ents, p->entity) != NULL);
    CHECK_EQ(EntityTable_Get(&ents, p->entity)->age_stage, LIFE_STAGE_BABY);
    Drain(NULL);
    for (d = 0; d < 200; d++) {
        const Entity* en;
        RunDays(&today, 1);
        en = EntityTable_GetConst(&ents, p->entity);
        CHECK(en != NULL);
        if (en) CHECK_EQ(en->age_stage, Life_StageOf(&L, p, today));      /* corpo sempre sincronizado */
        while (LifeEvents_Pop(&q, &e)) {
            if (e.type == LIFE_EVENT_STAGE_CHANGE) { stages_seen++; CHECK_EQ(e.arg, stages_seen); }
        }
    }
    CHECK_EQ(stages_seen, 6);                                    /* bebê -> idoso: 6 aniversários */
    CHECK_EQ(Life_StageOf(&L, p, today), LIFE_STAGE_ELDER);
    CHECK(Life_AgeDays(p, today) == 200u);

    /* envelhecimento desligado: a idade congela, sem eventos */
    Reset(7, today = 100);
    Life_SetSettings(&L, LIFE_SET_NATURAL_DEATH);
    a = Add("Fixo", 20, LIFE_GENDER_MALE, 0, 0, 1, 0, 0, today);
    p = Life_Get(&L, a);
    RunDays(&today, 100);
    CHECK_EQ(Life_AgeDays(p, today), 20);
    CHECK_EQ(Drain(counts), 0);
    CHECK(Lifecycle_Validate(&L));
    /* flag por pessoa */
    Reset(7, today = 100);
    a = Add("Fada", 20, LIFE_GENDER_OTHER, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today);
    Life_Get(&L, a)->flags |= LIFE_P_NO_AGING;
    RunDays(&today, 300);
    CHECK_EQ(Life_AgeDays(Life_Get(&L, a), today), 20);
    CHECK(Life_Get(&L, a)->flags & LIFE_P_ALIVE);
}

static void TestNaturalDeath(void)
{
    u32 seed, early = 0, at_cap = 0, today, d;
    PersonId a;

    for (seed = 1; seed <= 200; seed++) {
        Person* p;
        u32 es = Life_StageStartDays(LIFE_SPAN_NORMAL, LIFE_STAGE_ELDER);
        Reset(seed * 2654435761u, today = 5000);
        a = Add("Velho", es, LIFE_GENDER_MALE, 0, 0, 1, 0, 0, today);
        Life_MaterializeBodies(&L, &ctx, today);
        p = Life_Get(&L, a);
        for (d = 0; d < 200 && (p->flags & LIFE_P_ALIVE); d++) RunDays(&today, 1);
        CHECK(!(p->flags & LIFE_P_ALIVE));
        CHECK_EQ(p->cause, LIFE_CAUSE_OLD_AGE);
        CHECK(Life_AgeDays(p, (u32)p->death_day) <= es + 28u);           /* nunca passa do limite duro */
        CHECK(Life_AgeDays(p, (u32)p->death_day) >= es + 8u);            /* nunca antes do fim da graça */
        CHECK_EQ(p->entity, ENTITY_ID_NONE);
        CHECK_EQ(EntityTable_Count(&ents), 0);                           /* corpo removido */
        if (Life_AgeDays(p, (u32)p->death_day) == es + 28u) at_cap++; else early++;
        CHECK(Lifecycle_Validate(&L));
    }
    CHECK(early > 100);                                                  /* maioria morre antes do limite */
    CHECK(at_cap + early == 200u);

    /* sem morte natural: ninguém morre */
    Reset(3, today = 5000);
    Life_SetSettings(&L, LIFE_SET_AGING);
    a = Add("Imortal", 500, LIFE_GENDER_MALE, 0, 0, 0, 0, 0, today);
    RunDays(&today, 500);
    CHECK(Life_Get(&L, a)->flags & LIFE_P_ALIVE);
}

static void TestPregnancy(void)
{
    u32 today = 2000, counts[16], i, match = 0, total = 0, bits_hi = 0, bits_lo = 0;
    PersonId a, b, c, baby, kids[8];
    Person* pa;

    Reset(11, today);
    a = Add("Ana", Life_StageStartDays(LIFE_SPAN_NORMAL, LIFE_STAGE_YOUNG_ADULT), LIFE_GENDER_FEMALE, 0, 1, 1, 0, 0, today);
    b = Add("Bia", Life_StageStartDays(LIFE_SPAN_NORMAL, LIFE_STAGE_YOUNG_ADULT), LIFE_GENDER_MALE, 0, 0, 1, 0, 0, today);
    c = Add("Cris", Life_StageStartDays(LIFE_SPAN_NORMAL, LIFE_STAGE_YOUNG_ADULT), LIFE_GENDER_OTHER, 0, 1, 1, 0, 0, today);
    Life_MaterializeBodies(&L, &ctx, today);
    Drain(NULL);

    CHECK_EQ(Life_StartPregnancy(&L, &ctx, a, b, today), LIFE_ERR_RULE);          /* ainda não são parceiros */
    CHECK_EQ(Life_SetPartner(&L, &ctx, a, b, today), LIFE_OK);
    CHECK_EQ(Life_SetPartner(&L, &ctx, a, c, today), LIFE_ERR_RULE);              /* a já tem parceiro */
    CHECK_EQ(Life_StartPregnancy(&L, &ctx, b, a, today), LIFE_ERR_RULE);          /* b não gesta */
    CHECK_EQ(Life_StartPregnancy(&L, &ctx, a, a, today), LIFE_ERR_ARG);
    CHECK_EQ(Life_StartPregnancy(&L, &ctx, a, 0xFFFFu, today), LIFE_ERR_ARG);
    CHECK_EQ(Life_StartPregnancy(&L, &ctx, a, b, today), LIFE_OK);
    CHECK_EQ(Life_StartPregnancy(&L, &ctx, a, b, today), LIFE_ERR_STATE);         /* já gestante */
    pa = Life_Get(&L, a);
    CHECK_EQ(pa->preg_left, LIFE_PREGNANCY_DAYS);
    CHECK(Lifecycle_Validate(&L));

    RunDays(&today, LIFE_PREGNANCY_DAYS - 1u);
    CHECK_EQ(Life_GetChildren(&L, a, kids, 8), 0);                                 /* ainda não nasceu */
    RunDays(&today, 1);
    CHECK_EQ(Life_GetChildren(&L, a, kids, 8), 1);
    baby = kids[0];
    {
        const Person* bp = Life_GetConst(&L, baby);
        CHECK(bp != NULL && (bp->parent[0] == a && bp->parent[1] == b));
        CHECK_EQ(bp->birth_day, (s32)today);
        CHECK_EQ(Life_StageOf(&L, bp, today), LIFE_STAGE_BABY);
        CHECK_EQ(bp->household, 0);
        CHECK(bp->entity != ENTITY_ID_NONE && EntityTable_Get(&ents, bp->entity)->age_stage == LIFE_STAGE_BABY);
        CHECK_EQ(Life_Relationship(&L, baby, a), LIFE_REL_PARENT);
        CHECK_EQ(Life_Relationship(&L, baby, b), LIFE_REL_PARENT);
        CHECK(bp->flags & LIFE_P_ALIVE);
    }
    CHECK_EQ(pa->preg_left, 0);
    Drain(counts);
    CHECK_EQ(counts[LIFE_EVENT_BIRTH], 1);
    CHECK(Lifecycle_Validate(&L));

    /* herança: cada gene vem de um dos pais (exceto ~3% de mutação) */
    for (i = 0; i < 300; i++) {
        PersonId kid;
        const Person* kp;
        u32 k;
        Reset(1000u + i, today = 3000);
        a = Add("M", 38, LIFE_GENDER_FEMALE, 0, 1, 0, 0, 0, today);
        b = Add("P", 38, LIFE_GENDER_MALE, 0, 0, 0, 0, 0, today);
        Life_Get(&L, a)->genes = 0x12345678u; Life_Get(&L, b)->genes = 0x9ABCDEF0u;
        Life_Get(&L, a)->traits = 0xFFFFu;    Life_Get(&L, b)->traits = 0xFFFFu;
        Life_SetPartner(&L, &ctx, a, b, today);
        CHECK_EQ(Life_StartPregnancy(&L, &ctx, a, b, today), LIFE_OK);
        RunDays(&today, LIFE_PREGNANCY_DAYS);
        CHECK_EQ(Life_GetChildren(&L, a, &kid, 1), 1);
        kp = Life_GetConst(&L, kid);
        for (k = 0; k < 8; k++) {
            u32 n = (kp->genes >> (4u * k)) & 0xFu;
            u32 ga = (0x12345678u >> (4u * k)) & 0xFu, gb = (0x9ABCDEF0u >> (4u * k)) & 0xFu;
            total++; if (n == ga || n == gb) match++;
        }
        for (k = 0; k < 16; k++) if ((kp->traits >> k) & 1u) bits_hi++;
        /* sem traços nos pais: poucos traços aparecem */
        Life_Get(&L, a)->traits = 0; Life_Get(&L, b)->traits = 0;
    }
    CHECK(match * 100u >= total * 90u);                                            /* >= 90% herdado */
    CHECK(bits_hi > 300u * 12u);                                                   /* pais com tudo => filhos com quase tudo */
    (void)bits_lo;

    /* limite por família: 8 vivos (inclui gestações) */
    Reset(5, today = 3000);
    for (i = 0; i < LIFE_MAX_HOUSEHOLD_SIMS; i++) Add("x", 40, LIFE_GENDER_OTHER, 1, 1, 0, 0, 0, today);
    CHECK_EQ(AddR(NULL, "y", 40, LIFE_GENDER_OTHER, 1, 0, 0, 0, 0, today), LIFE_ERR_RULE);
    CHECK_EQ(AddR(NULL, "z", 40, LIFE_GENDER_OTHER, 2, 0, 0, 0, 0, today), LIFE_OK);        /* outra família */

    /* criança/idoso fora da faixa não gestam */
    Reset(5, today = 3000);
    a = Add("Crianca", 12, LIFE_GENDER_FEMALE, 0, 1, 0, 0, 0, today);
    b = Add("Adulto", 40, LIFE_GENDER_MALE, 0, 0, 0, 0, 0, today);
    CHECK_EQ(Life_SetPartner(&L, &ctx, a, b, today), LIFE_ERR_RULE);               /* criança não tem parceiro */
    CHECK_EQ(Life_StartPregnancy(&L, &ctx, a, b, today), LIFE_ERR_RULE);
}

static void TestDeferredBirth(void)
{
    u32 today = 4000, i, counts[16];
    PersonId a, b, filler[LIFE_MAX_PEOPLE], dummy;
    u32 nf = 0;

    /* 1) tabela de pessoas cheia e nada podável => adia; libera uma vaga => nasce */
    Reset(21, today);
    a = Add("Mae", 40, LIFE_GENDER_FEMALE, 0, 1, 1, 0, 0, today);
    b = Add("Pai", 40, LIFE_GENDER_MALE, 0, 0, 1, 0, 0, today);
    Life_MaterializeBodies(&L, &ctx, today);
    Life_SetPartner(&L, &ctx, a, b, today);
    CHECK_EQ(Life_StartPregnancy(&L, &ctx, a, b, today), LIFE_OK);
    while (nf < LIFE_MAX_PEOPLE - 2u) filler[nf++] = Add("f", 30, LIFE_GENDER_OTHER, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today);
    CHECK_EQ(AddR(&dummy, "extra", 30, 0, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today), LIFE_ERR_FULL);
    Drain(NULL);
    RunDays(&today, LIFE_PREGNANCY_DAYS);
    Drain(counts);
    CHECK_EQ(counts[LIFE_EVENT_BIRTH_DEFERRED], 1);                                 /* avisa uma vez */
    CHECK_EQ(Life_Get(&L, a)->preg_left, 1);
    CHECK(Life_Get(&L, a)->flags & LIFE_P_DEFERRED);
    RunDays(&today, 3);
    Drain(counts);
    CHECK_EQ(counts[LIFE_EVENT_BIRTH_DEFERRED], 0);                                 /* sem repetir o aviso */
    CHECK_EQ(Life_Kill(&L, &ctx, filler[0], LIFE_CAUSE_ACCIDENT, today), LIFE_OK);  /* morto e sem descendentes: podável */
    RunDays(&today, 1);
    Drain(counts);
    CHECK_EQ(counts[LIFE_EVENT_BIRTH], 1);
    CHECK_EQ(Life_Get(&L, a)->preg_left, 0);
    CHECK(!(Life_Get(&L, a)->flags & LIFE_P_DEFERRED));
    CHECK(Life_GetConst(&L, filler[0]) == NULL);                                    /* id velho não resolve mais */
    CHECK(Lifecycle_Validate(&L));

    /* 2) tabela de entidades cheia: o bebê nasce sem corpo e ganha um depois */
    Reset(22, today = 4000);
    a = Add("Mae", 40, LIFE_GENDER_FEMALE, 0, 1, 1, 0, 0, today);
    b = Add("Pai", 40, LIFE_GENDER_MALE, 0, 0, 1, 0, 0, today);
    Life_MaterializeBodies(&L, &ctx, today);
    Life_SetPartner(&L, &ctx, a, b, today);
    Life_StartPregnancy(&L, &ctx, a, b, today);
    {
        EntityId extra[OTS3_MAX_ENTITIES];
        u32 n = 0;
        Vec2Fx pos = { 0, 0 };
        while (EntityTable_Count(&ents) < OTS3_MAX_ENTITIES)
            extra[n++] = EntityTable_Spawn(&ents, ENTITY_KIND_SIM, 0, 3, 0, "d", pos);
        RunDays(&today, LIFE_PREGNANCY_DAYS);
        {
            PersonId kid;
            CHECK_EQ(Life_GetChildren(&L, a, &kid, 1), 1);
            CHECK_EQ(Life_GetConst(&L, kid)->entity, ENTITY_ID_NONE);
            CHECK(Life_GetConst(&L, kid)->flags & LIFE_P_WANTS_ENTITY);
            RunDays(&today, 2);
            CHECK_EQ(Life_GetConst(&L, kid)->entity, ENTITY_ID_NONE);               /* continua sem vaga */
            CHECK(EntityTable_Despawn(&ents, extra[0]));
            RunDays(&today, 1);
            CHECK(Life_GetConst(&L, kid)->entity != ENTITY_ID_NONE);                /* ganhou corpo */
            CHECK_EQ(EntityTable_Get(&ents, Life_GetConst(&L, kid)->entity)->age_stage, LIFE_STAGE_BABY);
        }
    }
    (void)i;
}

static void TestFamily(void)
{
    u32 today = 9000, i;
    PersonId ggp, gp1, gp2, p1, p2, h, p3, x, c1, c2, kids[8], many[40], root;
    LifeTree tree;

    Reset(31, today);
    ggp = Add("GGP", 400, LIFE_GENDER_MALE, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today);
    gp1 = Add("GP1", 300, LIFE_GENDER_MALE, LIFE_NO_HOUSEHOLD, 0, 0, ggp, 0, today);
    gp2 = Add("GP2", 300, LIFE_GENDER_FEMALE, LIFE_NO_HOUSEHOLD, 1, 0, 0, 0, today);
    p1 = Add("P1", 200, LIFE_GENDER_FEMALE, LIFE_NO_HOUSEHOLD, 1, 0, gp1, gp2, today);
    p2 = Add("P2", 190, LIFE_GENDER_MALE, LIFE_NO_HOUSEHOLD, 0, 0, gp1, gp2, today);
    h = Add("Half", 180, LIFE_GENDER_MALE, LIFE_NO_HOUSEHOLD, 0, 0, gp1, 0, today);
    p3 = Add("P3", 200, LIFE_GENDER_MALE, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today);
    x = Add("X", 200, LIFE_GENDER_FEMALE, LIFE_NO_HOUSEHOLD, 1, 0, 0, 0, today);
    c1 = Add("C1", 50, LIFE_GENDER_FEMALE, LIFE_NO_HOUSEHOLD, 0, 0, p1, p3, today);
    c2 = Add("C2", 40, LIFE_GENDER_MALE, LIFE_NO_HOUSEHOLD, 0, 0, p2, x, today);

    CHECK_EQ(Life_Relationship(&L, p1, gp1), LIFE_REL_PARENT);
    CHECK_EQ(Life_Relationship(&L, gp1, p1), LIFE_REL_CHILD);
    CHECK_EQ(Life_Relationship(&L, p1, p2), LIFE_REL_SIBLING);
    CHECK_EQ(Life_Relationship(&L, p1, h), LIFE_REL_HALF_SIBLING);
    CHECK_EQ(Life_Relationship(&L, c1, gp1), LIFE_REL_GRANDPARENT);
    CHECK_EQ(Life_Relationship(&L, gp1, c1), LIFE_REL_GRANDCHILD);
    CHECK_EQ(Life_Relationship(&L, c1, p2), LIFE_REL_AUNT_UNCLE);
    CHECK_EQ(Life_Relationship(&L, p2, c1), LIFE_REL_NIECE_NEPHEW);
    CHECK_EQ(Life_Relationship(&L, c1, c2), LIFE_REL_COUSIN);
    CHECK_EQ(Life_Relationship(&L, c1, ggp), LIFE_REL_ANCESTOR);
    CHECK_EQ(Life_Relationship(&L, ggp, c1), LIFE_REL_DESCENDANT);
    CHECK_EQ(Life_Relationship(&L, p1, x), LIFE_REL_UNRELATED);
    CHECK_EQ(Life_Relationship(&L, p1, p1), LIFE_REL_SELF);
    CHECK_EQ(Life_Relationship(&L, p1, 0xFFFFu), LIFE_REL_UNKNOWN);
    CHECK_EQ(Life_Relationship(&L, 0, p1), LIFE_REL_UNKNOWN);

    /* regras de parceria: parentes próximos não; primos e estranhos sim */
    CHECK(!Life_CanPartner(&L, p1, p2, today));                       /* irmãos */
    CHECK(!Life_CanPartner(&L, p1, h, today));                        /* meio-irmãos */
    CHECK(!Life_CanPartner(&L, c1, p2, today));                       /* tio */
    CHECK(!Life_CanPartner(&L, p1, c1, today));                       /* filha */
    CHECK(!Life_CanPartner(&L, c1, gp1, today));                      /* avô */
    CHECK(!Life_CanPartner(&L, c1, ggp, today));                      /* bisavô */
    CHECK(Life_CanPartner(&L, c1, c2, today) == (Life_StageCaps(Life_StageOf(&L, Life_GetConst(&L, c1), today)) & LIFE_CAP_PARTNER) && 1);
    CHECK(Life_CanPartner(&L, p1, p3, today));
    CHECK_EQ(Life_SetPartner(&L, NULL, p1, p3, today), LIFE_OK);
    CHECK_EQ(Life_Relationship(&L, p1, p3), LIFE_REL_PARTNER);
    CHECK_EQ(Life_Relationship(&L, p3, p1), LIFE_REL_PARTNER);
    CHECK(!Life_CanPartner(&L, p1, x, today));                        /* p1 já tem parceiro */
    CHECK_EQ(Life_ClearPartner(&L, NULL, p1), LIFE_OK);
    CHECK_EQ(Life_Get(&L, p3)->partner, 0);
    CHECK_EQ(Life_ClearPartner(&L, NULL, p1), LIFE_ERR_STATE);

    /* criação: pai mais novo que o filho (ciclo) é recusado */
    CHECK_EQ(AddR(NULL, "Ciclo", 500, 0, LIFE_NO_HOUSEHOLD, 0, 0, c1, 0, today), LIFE_ERR_RULE);
    CHECK_EQ(AddR(NULL, "Igual", 300, 0, LIFE_NO_HOUSEHOLD, 0, 0, gp1, 0, today), LIFE_ERR_RULE);   /* mesmo dia */
    CHECK_EQ(AddR(NULL, "Dup", 10, 0, LIFE_NO_HOUSEHOLD, 0, 0, gp1, gp1, today), LIFE_ERR_ARG);
    CHECK_EQ(AddR(NULL, "Inex", 10, 0, LIFE_NO_HOUSEHOLD, 0, 0, 0x7777u, 0, today), LIFE_ERR_ARG);

    /* filhos em ordem de nascimento */
    CHECK_EQ(Life_GetChildren(&L, gp1, kids, 8), 3);
    CHECK(kids[0] == p1 && kids[1] == p2 && kids[2] == h);
    CHECK_EQ(Life_GetChildren(&L, gp1, kids, 2), 2);                  /* respeita a capacidade */
    CHECK(kids[0] == p1 && kids[1] == p2);
    CHECK_EQ(Life_GetChildren(&L, c1, kids, 8), 0);

    /* árvores */
    CHECK_EQ(Life_BuildTree(&L, c1, LIFE_TREE_ANCESTORS, 3, &tree), 6);
    CHECK(tree.node[0].id == c1 && tree.node[0].parent_node == -1 && tree.node[0].depth == 0);
    CHECK(tree.node[1].id == p1 && tree.node[2].id == p3 && tree.node[1].depth == 1);
    CHECK(tree.node[3].id == gp1 && tree.node[4].id == gp2 && tree.node[3].depth == 2);
    CHECK(tree.node[5].id == ggp && tree.node[5].depth == 3 && tree.node[5].parent_node == 3);
    CHECK_EQ(Life_BuildTree(&L, c1, LIFE_TREE_ANCESTORS, 1, &tree), 3);
    CHECK_EQ(Life_BuildTree(&L, gp1, LIFE_TREE_DESCENDANTS, 4, &tree), 6);      /* gp1,p1,p2,h,c1,c2 */
    CHECK_EQ(Life_BuildTree(&L, 0x4242u, LIFE_TREE_ANCESTORS, 3, &tree), 0);
    /* árvore larga é cortada em LIFE_TREE_MAX, sem estourar */
    Reset(32, today);
    root = Add("Raiz", 400, 0, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today);
    for (i = 0; i < 40; i++) many[i] = Add("k", 10, 0, LIFE_NO_HOUSEHOLD, 0, 0, root, 0, today);
    CHECK_EQ(Life_BuildTree(&L, root, LIFE_TREE_DESCENDANTS, 5, &tree), LIFE_TREE_MAX);
    CHECK_EQ(tree.truncated, 1);
    CHECK_EQ(Life_GetChildren(&L, root, many, 40), 40);
    CHECK(Lifecycle_Validate(&L));
}

static void TestDeathAndPets(void)
{
    u32 today = 7000, counts[16];
    PersonId a, b, child;
    Person* pa;
    EntityId pet1, pet2, bent, aent;
    Vec2Fx pos = { FX_TILE_CENTER(2), FX_TILE_CENTER(2) };

    Reset(41, today);
    a = Add("A", 40, LIFE_GENDER_FEMALE, 0, 1, 1, 0, 0, today);
    b = Add("B", 40, LIFE_GENDER_MALE, 0, 0, 1, 0, 0, today);
    child = Add("Cria", 15, LIFE_GENDER_MALE, 0, 0, 1, a, b, today);
    Life_MaterializeBodies(&L, &ctx, today);
    Life_SetPartner(&L, &ctx, a, b, today);
    pa = Life_Get(&L, a);
    aent = pa->entity; bent = Life_Get(&L, b)->entity;
    CHECK_EQ(EntityTable_Get(&ents, Life_Get(&L, child)->entity)->age_stage, LIFE_STAGE_CHILD);
    pet1 = EntityTable_Spawn(&ents, ENTITY_KIND_PET, PET_SPECIES_DOG, 0, aent, "Rex", pos);
    pet2 = EntityTable_Spawn(&ents, ENTITY_KIND_PET, PET_SPECIES_CAT, 0, bent, "Mia", pos);
    Drain(NULL);

    CHECK_EQ(Life_Kill(&L, &ctx, a, LIFE_CAUSE_NONE, today), LIFE_ERR_ARG);
    CHECK_EQ(Life_Kill(&L, &ctx, a, LIFE_CAUSE_COUNT, today), LIFE_ERR_ARG);
    CHECK_EQ(Life_Kill(&L, &ctx, a, LIFE_CAUSE_STARVATION, today), LIFE_OK);
    CHECK(!(pa->flags & LIFE_P_ALIVE) && pa->cause == LIFE_CAUSE_STARVATION && pa->death_day == (s32)today);
    CHECK_EQ(pa->entity, ENTITY_ID_NONE);
    CHECK(EntityTable_Get(&ents, aent) == NULL);                                    /* corpo removido */
    CHECK_EQ(Life_Get(&L, b)->partner, 0);                                           /* viuvez */
    CHECK_EQ(EntityTable_Get(&ents, pet1)->owner, bent);                             /* pet foi para o herdeiro (B) */
    CHECK_EQ(EntityTable_Get(&ents, pet2)->owner, bent);                             /* o de B não mudou */
    Drain(counts);
    CHECK_EQ(counts[LIFE_EVENT_DEATH], 1);
    CHECK_EQ(Life_Kill(&L, &ctx, a, LIFE_CAUSE_ACCIDENT, today), LIFE_ERR_STATE);    /* idempotente */
    CHECK_EQ(Life_SetPartner(&L, &ctx, a, b, today), LIFE_ERR_STATE);                /* morto não casa */
    CHECK_EQ(Life_StartPregnancy(&L, &ctx, a, b, today), LIFE_ERR_STATE);
    CHECK_EQ(Life_Relationship(&L, child, a), LIFE_REL_PARENT);                      /* genealogia sobrevive à morte */
    CHECK(Lifecycle_Validate(&L));

    /* sem herdeiro: pets ficam sem dono (nunca com id velho) */
    CHECK_EQ(Life_Kill(&L, &ctx, b, LIFE_CAUSE_OTHER, today), LIFE_OK);              /* B morre; só a criança resta */
    CHECK_EQ(EntityTable_Get(&ents, pet1)->owner, Life_Get(&L, child)->entity);      /* criança (>= CHILD) herda */
    CHECK_EQ(Life_Kill(&L, &ctx, child, LIFE_CAUSE_OTHER, today), LIFE_OK);
    CHECK_EQ(EntityTable_Get(&ents, pet1)->owner, ENTITY_ID_NONE);
    CHECK_EQ(EntityTable_Get(&ents, pet2)->owner, ENTITY_ID_NONE);
    CHECK_EQ(Life_CountAlive(&L), 0);
}

static void TestPruneAndGenerations(void)
{
    u32 today = 8000, i;
    PersonId ids[LIFE_MAX_PEOPLE], gg, gp, par, kid, fresh;

    /* ancestrais de vivos (até 3 gerações) nunca são podados; os demais mortos, sim */
    Reset(51, today);
    gg = Add("GG", 300, 0, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today);
    gp = Add("GP", 250, 0, LIFE_NO_HOUSEHOLD, 0, 0, gg, 0, today);
    par = Add("PAR", 200, 0, LIFE_NO_HOUSEHOLD, 0, 0, gp, 0, today);
    kid = Add("KID", 5, 0, LIFE_NO_HOUSEHOLD, 0, 0, par, 0, today);
    Life_Kill(&L, &ctx, gg, LIFE_CAUSE_OLD_AGE, today - 300);
    Life_Kill(&L, &ctx, gp, LIFE_CAUSE_OLD_AGE, today - 200);
    Life_Kill(&L, &ctx, par, LIFE_CAUSE_OLD_AGE, today - 100);
    for (i = 0; i < LIFE_MAX_PEOPLE - 4u; i++) {
        ids[i] = Add("t", 100, 0, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today);
        if (i < 10) Life_Kill(&L, &ctx, ids[i], LIFE_CAUSE_ACCIDENT, today - 50 + i);      /* i=0 é a mais antiga */
    }
    CHECK_EQ(Life_CountAlive(&L), LIFE_MAX_PEOPLE - 4u - 10u + 1u);
    CHECK_EQ(AddR(&fresh, "Novo", 1, 0, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today), LIFE_OK);   /* poda a mais antiga */
    CHECK(Life_GetConst(&L, ids[0]) == NULL);                                                /* id velho morreu */
    CHECK(Life_GetConst(&L, ids[1]) != NULL);
    CHECK(Life_GetConst(&L, gg) != NULL && Life_GetConst(&L, gp) != NULL && Life_GetConst(&L, par) != NULL);
    CHECK(fresh != ids[0] && PERSON_ID_INDEX(fresh) == PERSON_ID_INDEX(ids[0]));             /* slot reaproveitado... */
    CHECK_EQ(PERSON_ID_GEN(fresh), PERSON_ID_GEN(ids[0]) + 1u);                              /* ...com outra geração */
    CHECK_EQ(Life_Relationship(&L, kid, gg), LIFE_REL_ANCESTOR);
    CHECK(Lifecycle_Validate(&L));
    /* esgota as podáveis; as protegidas ficam e a tabela acaba cheia (erro, não corrupção) */
    {
        u32 guard = 0;
        while (guard++ < 200u && AddR(NULL, "q", 1, 0, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today) == LIFE_OK) { }
        CHECK_EQ(AddR(NULL, "q", 1, 0, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today), LIFE_ERR_FULL);
        CHECK(Life_GetConst(&L, gg) != NULL && Life_GetConst(&L, par) != NULL);
        CHECK(Lifecycle_Validate(&L));
    }
    /* pai podado vira "desconhecido": o filho continua válido */
    Reset(52, today);
    par = Add("PAR", 200, 0, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today);
    kid = Add("KID", 5, 0, LIFE_NO_HOUSEHOLD, 0, 0, par, 0, today);
    Life_Kill(&L, &ctx, kid, LIFE_CAUSE_ACCIDENT, today);          /* nenhum vivo mantém 'par' protegido */
    Life_Kill(&L, &ctx, par, LIFE_CAUSE_ACCIDENT, today);
    for (i = 0; i < LIFE_MAX_PEOPLE; i++) AddR(NULL, "w", 1, 0, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today);
    CHECK(Life_GetConst(&L, par) == NULL);
    CHECK(Lifecycle_Validate(&L));
}

static void TestCatchupAndGuards(void)
{
    u32 today = 100000, done;
    PersonId a, b;

    Reset(61, today);
    a = Add("A", 50, 0, 0, 0, 1, 0, 0, today);
    Life_MaterializeBodies(&L, &ctx, today);
    CHECK_EQ(Life_Update(&L, &ctx, today), 0);                                      /* mesmo dia: nada */
    CHECK_EQ(Life_Update(&L, &ctx, today - 5u), 0);                                 /* relógio voltou: nada */
    CHECK_EQ(L.last_day, today);
    done = Life_Update(&L, &ctx, today + 1000u);
    CHECK_EQ(done, LIFE_MAX_CATCHUP_DAYS);                                           /* limitado: não trava */
    CHECK_EQ(L.last_day, today + 1000u);
    CHECK_EQ(Life_AgeDays(Life_Get(&L, a), today + 1000u), 1050);                    /* idade é derivada: correta */
    CHECK_EQ(Life_Update(&L, &ctx, LIFE_MAX_DAY + 1u), 0);                           /* fora da faixa: ignora */
    CHECK_EQ(Life_Update(&L, NULL, today + 2000u), 0);
    CHECK_EQ(Life_Update(NULL, &ctx, today + 2000u), 0);

    /* idade congelada sobrevive ao salto */
    Reset(62, today = 100000);
    Life_SetSettings(&L, LIFE_SET_NATURAL_DEATH);
    b = Add("B", 50, 0, 0, 0, 0, 0, 0, today);
    Life_Update(&L, &ctx, today + 5000u);
    CHECK_EQ(Life_AgeDays(Life_Get(&L, b), today + 5000u), 50);
    CHECK(Lifecycle_Validate(&L));

    /* argumentos inválidos */
    Reset(63, today = 500);
    CHECK_EQ(AddR(NULL, "x", 1, LIFE_GENDER_COUNT, 0, 0, 0, 0, 0, today), LIFE_ERR_ARG);
    CHECK_EQ(AddR(NULL, "x", 1, 0, 9, 0, 0, 0, 0, today), LIFE_ERR_ARG);
    CHECK_EQ(AddR(NULL, "x", 1, 0, LIFE_NO_HOUSEHOLD, 0, 1, 0, 0, today), LIFE_ERR_ARG);   /* corpo sem família */
    CHECK_EQ(AddR(NULL, "x", LIFE_MAX_DAY + 1u, 0, 0, 0, 0, 0, 0, today), LIFE_ERR_ARG);
    CHECK_EQ(AddR(NULL, "x", 1, 0, 0, 0, 0, 0, 0, LIFE_MAX_DAY + 1u), LIFE_ERR_ARG);
    CHECK_EQ(Life_AddPerson(&L, NULL, today, NULL), LIFE_ERR_ARG);
    CHECK_EQ(Life_SetName(&L, 0x1234u, "x"), LIFE_ERR_ARG);
    a = Add("NomeGigantescoDemais", 1, 0, 0, 0, 0, 0, 0, today);
    CHECK_EQ(strlen(Life_Get(&L, a)->name), OTS3_NAME_LEN - 1u);
    CHECK_EQ(Life_SetName(&L, a, "Zeca"), LIFE_OK);
    CHECK(strcmp(Life_Get(&L, a)->name, "Zeca") == 0);
    CHECK_EQ(Life_SetWantsEntity(&L, a, TRUE), LIFE_OK);
    CHECK(Life_Get(&L, a)->flags & LIFE_P_WANTS_ENTITY);
    CHECK(Life_Get(&L, 0x9999u) == NULL && Life_GetConst(&L, 0) == NULL);
}

static void TestEventsAndHooks(void)
{
    u32 today = 600, i, n = 0;
    PersonId a, b;
    LifeHooks hooks;
    LifeEvent e;

    /* fila: FIFO, capacidade fixa, descarta o NOVO e conta */
    Reset(71, today);
    a = Add("A", 40, 0, 0, 0, 0, 0, 0, today);
    b = Add("B", 40, 0, 0, 0, 0, 0, 0, today);
    for (i = 0; i < 30; i++) {
        CHECK_EQ(Life_SetPartner(&L, &ctx, a, b, today), LIFE_OK);
        CHECK_EQ(Life_ClearPartner(&L, &ctx, a), LIFE_OK);
    }
    CHECK_EQ(q.count, LIFE_EVENT_CAP);
    CHECK_EQ(q.dropped, 60 - LIFE_EVENT_CAP);
    CHECK(LifeEvents_Pop(&q, &e) && e.type == LIFE_EVENT_PARTNER && e.person == a && e.other == b);
    CHECK(LifeEvents_Pop(&q, &e) && e.type == LIFE_EVENT_BREAKUP);
    while (LifeEvents_Pop(&q, &e)) n++;
    CHECK_EQ(n, LIFE_EVENT_CAP - 2u);
    CHECK(!LifeEvents_Pop(&q, &e));
    ctx.events = NULL;                                                                /* sem fila: tudo continua funcionando */
    CHECK_EQ(Life_SetPartner(&L, &ctx, a, b, today), LIFE_OK);
    ctx.events = &q;

    /* hooks: criação e remoção passam pela simulação, não direto pela tabela */
    Reset(72, today = 600);
    hooks.user = NULL; hooks.spawn_sim = HookSpawn; hooks.despawn = HookDespawn;
    ctx.hooks = &hooks;
    gSpawnCalls = gDespawnCalls = 0;
    a = Add("A", 40, LIFE_GENDER_FEMALE, 0, 1, 1, 0, 0, today);
    b = Add("B", 40, LIFE_GENDER_MALE, 0, 0, 1, 0, 0, today);
    CHECK_EQ(Life_MaterializeBodies(&L, &ctx, today), 2);
    CHECK_EQ(gSpawnCalls, 2);
    Life_SetPartner(&L, &ctx, a, b, today);
    Life_StartPregnancy(&L, &ctx, a, b, today);
    RunDays(&today, LIFE_PREGNANCY_DAYS);
    CHECK_EQ(gSpawnCalls, 3);                                                          /* o bebê */
    CHECK_EQ(Life_Kill(&L, &ctx, b, LIFE_CAUSE_ACCIDENT, today), LIFE_OK);
    CHECK_EQ(gDespawnCalls, 1);
    CHECK(Lifecycle_Validate(&L));
}

/* ------------------------------------------------------- serialização -- */
static u8 gBuf[LIFE_SERIAL_MAX];
static Lifecycle gSink, gOther;

static void BuildRichState(void)
{
    u32 today = 12000;
    PersonId a, b, c, d, e;
    Reset(81, today);
    a = Add("Ana", 40, LIFE_GENDER_FEMALE, 0, 1, 1, 0, 0, today);
    b = Add("Beto", 41, LIFE_GENDER_MALE, 0, 0, 1, 0, 0, today);
    c = Add("Cida", 90, LIFE_GENDER_OTHER, 1, 0, 0, 0, 0, today);
    d = Add("Dino", 12, LIFE_GENDER_MALE, 0, 0, 1, a, b, today);
    e = Add("Edu", 70, LIFE_GENDER_MALE, LIFE_NO_HOUSEHOLD, 0, 0, 0, 0, today);
    Life_MaterializeBodies(&L, &ctx, today);
    Life_SetPartner(&L, &ctx, a, b, today);
    Life_StartPregnancy(&L, &ctx, a, b, today);
    Life_Kill(&L, &ctx, c, LIFE_CAUSE_OLD_AGE, today);
    Life_Kill(&L, &ctx, e, LIFE_CAUSE_ACCIDENT, today);
    Life_Get(&L, d)->traits = 0xBEEFu;
    (void)d;
}

static void TestSerialization(void)
{
    u32 len = 0, len2 = 0, i, bit, iter, ok_count = 0, bad_count = 0;
    static u8 buf2[LIFE_SERIAL_MAX];
    static u8 mut[LIFE_SERIAL_MAX];

    BuildRichState();
    CHECK(Lifecycle_Validate(&L));
    CHECK(Lifecycle_Serialize(&L, gBuf, sizeof(gBuf), &len));
    CHECK(len > 100u && len <= LIFE_SERIAL_MAX);
    CHECK(!Lifecycle_Serialize(&L, gBuf, len - 1u, &len2));                           /* buffer curto: erro, sem escrever além */
    CHECK(!Lifecycle_Serialize(&L, gBuf, 0, &len2));

    /* ida e volta: estado idêntico (comparado pela forma canônica) */
    memset(&gOther, 0, sizeof(gOther));
    CHECK_EQ(Lifecycle_Deserialize(&gOther, gBuf, len), LIFE_OK);
    CHECK(Lifecycle_Serialize(&gOther, buf2, sizeof(buf2), &len2));
    CHECK(len2 == len && memcmp(gBuf, buf2, len) == 0);
    CHECK(Lifecycle_Validate(&gOther));
    CHECK_EQ(Life_CountAlive(&gOther), Life_CountAlive(&L));

    /* truncamentos e lixo no fim: sempre recusados, e o destino não muda */
    for (i = 0; i < len; i++) {
        memset(&gSink, 0xAB, sizeof(gSink));
        CHECK_EQ(Lifecycle_Deserialize(&gSink, gBuf, i), LIFE_ERR_CORRUPT);
        CHECK(((u8*)&gSink)[0] == 0xAB && ((u8*)&gSink)[sizeof(gSink) - 1u] == 0xAB);
    }
    memcpy(mut, gBuf, len); mut[len] = 0;
    CHECK_EQ(Lifecycle_Deserialize(&gSink, mut, len + 1u), LIFE_ERR_CORRUPT);
    CHECK_EQ(Lifecycle_Deserialize(NULL, gBuf, len), LIFE_ERR_ARG);
    CHECK_EQ(Lifecycle_Deserialize(&gSink, NULL, len), LIFE_ERR_ARG);

    /* todo bit invertido: nunca derruba; se aceitar, o resultado é um estado VÁLIDO */
    for (i = 0; i < len; i++) {
        for (bit = 0; bit < 8; bit++) {
            LifeResult r;
            memcpy(mut, gBuf, len); mut[i] ^= (u8)(1u << bit);
            memset(&gSink, 0xAB, sizeof(gSink));
            r = Lifecycle_Deserialize(&gSink, mut, len);
            if (r == LIFE_OK) { CHECK(Lifecycle_Validate(&gSink)); ok_count++; }
            else { CHECK_EQ(r, LIFE_ERR_CORRUPT); CHECK_EQ(((u8*)&gSink)[100], 0xAB); bad_count++; }
        }
    }
    CHECK(bad_count > ok_count / 4u);                                                  /* a validação rejeita bastante */

    /* fuzz: bytes aleatórios em posições aleatórias */
    for (iter = 0; iter < 30000; iter++) {
        u32 n = 1u + Test_Rand() % 6u, k;
        memcpy(mut, gBuf, len);
        for (k = 0; k < n; k++) mut[Test_Rand() % len] = (u8)Test_Rand();
        if (Lifecycle_Deserialize(&gSink, mut, len) == LIFE_OK) CHECK(Lifecycle_Validate(&gSink));
    }
    /* lixo puro */
    for (iter = 0; iter < 5000; iter++) {
        u32 n = Test_Rand() % LIFE_SERIAL_MAX, k;
        for (k = 0; k < n; k++) mut[k] = (u8)Test_Rand();
        mut[0] = LIFE_SERIAL_VERSION;
        if (Lifecycle_Deserialize(&gSink, mut, n) == LIFE_OK) CHECK(Lifecycle_Validate(&gSink));
    }
}

/* estados maliciosos com formato válido: só a validação os pega */
static void TestMalicious(void)
{
    u32 today = 12000, len;
    PersonId a, b;
    Lifecycle bad;
    Person* pa;
    Person* pb;

    #define MAKE_BAD() do { BuildRichState(); a = Life_Get(&L, 0) ? 0 : 0; } while (0)
    BuildRichState();
    {
        PersonId first = 0, second = 0;
        u32 i;
        for (i = 0; i < LIFE_MAX_PEOPLE; i++) if (L.people[i].id) { if (!first) first = L.people[i].id; else if (!second) second = L.people[i].id; }
        a = first; b = second;
    }
    memcpy(&bad, &L, sizeof(bad));
    CHECK(Lifecycle_Validate(&bad));

    /* ciclo de pais (mesmo dia de nascimento) */
    memcpy(&bad, &L, sizeof(bad)); pa = Life_Get(&bad, a); pb = Life_Get(&bad, b);
    pa->parent[0] = b; pb->parent[0] = a;
    CHECK(!Lifecycle_Validate(&bad));
    /* auto-referência */
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, a)->parent[0] = a;
    CHECK(!Lifecycle_Validate(&bad));
    /* mesmo pai duas vezes */
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, b)->parent[0] = Life_Get(&bad, a)->id; Life_Get(&bad, b)->parent[1] = Life_Get(&bad, a)->id;
    CHECK(!Lifecycle_Validate(&bad));
    /* parceria não mútua */
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, a)->partner = Life_Get(&bad, b)->id; Life_Get(&bad, b)->partner = 0;
    CHECK(!Lifecycle_Validate(&bad));
    /* parceria consigo mesmo */
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, a)->partner = a;
    CHECK(!Lifecycle_Validate(&bad));
    /* dois donos do mesmo corpo */
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, a)->entity = 0x00010003u; Life_Get(&bad, b)->entity = 0x00010003u;
    CHECK(!Lifecycle_Validate(&bad));
    /* morto com parceiro / corpo / gestação */
    {
        PersonId dead = 0; u32 i;
        for (i = 0; i < LIFE_MAX_PEOPLE; i++) if (L.people[i].id && !(L.people[i].flags & LIFE_P_ALIVE)) dead = L.people[i].id;
        CHECK(dead != 0);
        memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, dead)->partner = a;     CHECK(!Lifecycle_Validate(&bad));
        memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, dead)->entity = 5;      CHECK(!Lifecycle_Validate(&bad));
        memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, dead)->preg_left = 1;   CHECK(!Lifecycle_Validate(&bad));
        memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, dead)->cause = 0;       CHECK(!Lifecycle_Validate(&bad));
        memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, dead)->death_day = -5;  CHECK(!Lifecycle_Validate(&bad));
    }
    /* vivo com causa de morte */
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, a)->cause = LIFE_CAUSE_OLD_AGE; CHECK(!Lifecycle_Validate(&bad));
    /* gestação fora da faixa / sem poder gestar / flag órfã */
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, b)->preg_left = 1; CHECK(!Lifecycle_Validate(&bad));      /* b não gesta */
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, a)->preg_left = 200; CHECK(!Lifecycle_Validate(&bad));
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, b)->flags |= LIFE_P_DEFERRED; CHECK(!Lifecycle_Validate(&bad));
    /* campos fora da faixa */
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, a)->birth_day = 0x7FFFFFFF; CHECK(!Lifecycle_Validate(&bad));
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, a)->household = 77; CHECK(!Lifecycle_Validate(&bad));
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, a)->gender = 9; CHECK(!Lifecycle_Validate(&bad));
    memcpy(&bad, &L, sizeof(bad)); Life_Get(&bad, a)->flags |= 0x80; CHECK(!Lifecycle_Validate(&bad));
    memcpy(&bad, &L, sizeof(bad)); memset(Life_Get(&bad, a)->name, 'z', OTS3_NAME_LEN); CHECK(!Lifecycle_Validate(&bad));
    memcpy(&bad, &L, sizeof(bad)); bad.gen[3] = 0; CHECK(!Lifecycle_Validate(&bad));
    memcpy(&bad, &L, sizeof(bad)); bad.rng = 0; CHECK(!Lifecycle_Validate(&bad));
    memcpy(&bad, &L, sizeof(bad)); bad.lifespan = 9; CHECK(!Lifecycle_Validate(&bad));
    memcpy(&bad, &L, sizeof(bad)); bad.settings = 0xF0; CHECK(!Lifecycle_Validate(&bad));
    memcpy(&bad, &L, sizeof(bad)); bad.people[0].id = PERSON_ID_MAKE(2, 1); CHECK(!Lifecycle_Validate(&bad));   /* id x slot */
    /* e o conjunto sem alterações continua válido (os testes acima não vieram de falso positivo) */
    memcpy(&bad, &L, sizeof(bad)); CHECK(Lifecycle_Validate(&bad));

    /* cabeçalho serializado malicioso (contagem, slots repetidos/fora de ordem, versão) */
    CHECK(Lifecycle_Serialize(&L, gBuf, sizeof(gBuf), &len));
    gBuf[3] = 200;          CHECK_EQ(Lifecycle_Deserialize(&gSink, gBuf, len), LIFE_ERR_CORRUPT);   /* count > máx */
    CHECK(Lifecycle_Serialize(&L, gBuf, sizeof(gBuf), &len));
    gBuf[0] = 9;            CHECK_EQ(Lifecycle_Deserialize(&gSink, gBuf, len), LIFE_ERR_CORRUPT);   /* versão */
    CHECK(Lifecycle_Serialize(&L, gBuf, sizeof(gBuf), &len));
    gBuf[12 + LIFE_MAX_PEOPLE] = 200;                                                                /* slot >= máx */
    CHECK_EQ(Lifecycle_Deserialize(&gSink, gBuf, len), LIFE_ERR_CORRUPT);
    CHECK(Lifecycle_Serialize(&L, gBuf, sizeof(gBuf), &len));
    gBuf[3] = (u8)(gBuf[3] + 1u);                                                                    /* count maior que os dados */
    CHECK_EQ(Lifecycle_Deserialize(&gSink, gBuf, len), LIFE_ERR_CORRUPT);
    (void)today;
}

/* -------------------------------------------------------------- dinastia -- */

static u32 gBirths, gDeaths, gFull;

static void CheckInvariants(u32 today)
{
    u32 i, h, load[OTS3_MAX_HOUSEHOLDS];

    CHECK(Lifecycle_Validate(&L));
    memset(load, 0, sizeof(load));
    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
        const Person* p = &L.people[i];
        if (!p->id || !(p->flags & LIFE_P_ALIVE)) continue;
        if (p->household < OTS3_MAX_HOUSEHOLDS) load[p->household] += 1u + (p->preg_left ? 1u : 0u);
        if (p->entity != ENTITY_ID_NONE) {
            const Entity* e = EntityTable_GetConst(&ents, p->entity);
            if (e) {
                CHECK(e->kind == ENTITY_KIND_SIM && e->household == p->household);
                CHECK_EQ(e->age_stage, Life_StageOf(&L, p, today));
            }
        }
    }
    for (h = 0; h < OTS3_MAX_HOUSEHOLDS; h++) CHECK(load[h] <= LIFE_MAX_HOUSEHOLD_SIMS);
}

static u32 RunDynasty(u32 seed, u32 days)
{
    u32 today = 20000, d, i, h, hash = 2166136261u;
    u32 st[LIFE_STAGE_COUNT];
    static u8 snap[LIFE_SERIAL_MAX];
    u32 snap_len;

    g_lcg = seed;
    Reset(seed, today);
    gBirths = gDeaths = gFull = 0;
    for (h = 0; h < 2; h++) {
        Add("F", Life_StageStartDays(LIFE_SPAN_NORMAL, LIFE_STAGE_YOUNG_ADULT), LIFE_GENDER_FEMALE, (u8)h, 1, 1, 0, 0, today);
        Add("M", Life_StageStartDays(LIFE_SPAN_NORMAL, LIFE_STAGE_YOUNG_ADULT), LIFE_GENDER_MALE, (u8)h, 0, 1, 0, 0, today);
    }
    Life_MaterializeBodies(&L, &ctx, today);
    memset(st, 0, sizeof(st));
    for (d = 0; d < days; d++) {
        LifeEvent e;
        today++;
        Life_Update(&L, &ctx, today);
        while (LifeEvents_Pop(&q, &e)) {
            if (e.type == LIFE_EVENT_BIRTH) gBirths++;
            if (e.type == LIFE_EVENT_DEATH) gDeaths++;
            if (e.type == LIFE_EVENT_STAGE_CHANGE && e.arg < LIFE_STAGE_COUNT) st[e.arg]++;
        }
        /* política simples: imigrante a cada 40 dias, casais se formam, gestações tentadas */
        if (d % 40u == 0) {
            LifePersonInit in;
            memset(&in, 0, sizeof(in));
            in.name = "Imig"; in.age_days = Life_StageStartDays(LIFE_SPAN_NORMAL, LIFE_STAGE_YOUNG_ADULT);
            in.household = (u8)(Test_Rand() % 2u); in.wants_entity = 1;
            in.gender = (u8)(Test_Rand() % 2u); in.can_carry = (u8)(in.gender == LIFE_GENDER_FEMALE);
            if (Life_AddPerson(&L, &in, today, NULL) == LIFE_ERR_FULL) gFull++;
            Life_MaterializeBodies(&L, &ctx, today);
        }
        for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
            Person* p = &L.people[i];
            u32 j;
            if (!p->id || !(p->flags & LIFE_P_ALIVE) || !(Life_StageCaps(Life_StageOf(&L, p, today)) & LIFE_CAP_PARTNER)) continue;
            if (p->partner == PERSON_ID_NONE) {
                for (j = i + 1u; j < LIFE_MAX_PEOPLE; j++) {
                    Person* o = &L.people[j];
                    if (o->id && o->household == p->household && Life_CanPartner(&L, p->id, o->id, today)) {
                        (void)Life_SetPartner(&L, &ctx, p->id, o->id, today);
                        break;
                    }
                }
            } else if (p->preg_left == 0 && (p->flags & LIFE_P_CAN_CARRY) && Test_Rand() % 100u < 20u) {
                (void)Life_StartPregnancy(&L, &ctx, p->id, p->partner, today);
            }
        }
        CheckInvariants(today);
        if (d % 100u == 99u) {
            CHECK(Lifecycle_Serialize(&L, snap, sizeof(snap), &snap_len));
            hash = (hash ^ Fnv(snap, snap_len)) * 16777619u;
        }
    }
    for (i = 1; i < LIFE_STAGE_COUNT; i++) CHECK(st[i] > 0);          /* todas as fases foram vividas por alguém */
    return hash;
}

static void TestDynasty(void)
{
    u32 h1, h2, h3;

    h1 = RunDynasty(1234u, 4000);
    CHECK(gBirths > 15u && gDeaths > 15u);
    CHECK_EQ(gFull, 0);                                                /* a poda mantém a tabela com vaga */
    h2 = RunDynasty(1234u, 4000);
    CHECK_EQ(h1, h2);                                                  /* mesma semente => mesma história, bit a bit */
    h3 = RunDynasty(4321u, 4000);
    CHECK(h3 != h1);                                                   /* outra semente => outra história */
}

int main(void)
{
    TestStages();
    TestAging();
    TestNaturalDeath();
    TestPregnancy();
    TestDeferredBirth();
    TestFamily();
    TestDeathAndPets();
    TestPruneAndGenerations();
    TestCatchupAndGuards();
    TestEventsAndHooks();
    TestSerialization();
    TestMalicious();
    TestDynasty();
    return Test_Finish("lifecycle");
}
