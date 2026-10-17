#include "entity.h"

#include <string.h>

/* Taxas de decaimento em unidades (de 1000) por hora de jogo. 0 = não se aplica.
 * Valores de projeto (ajustáveis), não extraídos do jogo original.
 * Ex.: fome 40/h => 25 horas do cheio ao vazio. */
static const u8 kDecaySim[NEED_COUNT] = {
    /* HUNGER */ 40, /* ENERGY */ 30, /* AFFECTION */ 0,
    /* BLADDER */ 55, /* HYGIENE */ 28, /* FUN */ 30
};
static const u8 kDecayPet[NEED_COUNT] = {
    /* HUNGER */ 36, /* ENERGY */ 28, /* AFFECTION */ 22,
    /* BLADDER */ 0, /* HYGIENE */ 0, /* FUN */ 0
};

#define NEED_START_VALUE 850

static const u8* DecayTable(u8 kind)
{
    return kind == ENTITY_KIND_PET ? kDecayPet : kDecaySim;
}

/* ------------------------------------------------------------------ tabela -- */

void EntityTable_Init(EntityTable* t)
{
    u32 i;

    memset(t, 0, sizeof(*t));
    for (i = 0; i < OTS3_MAX_ENTITIES; i++) {
        t->gen[i] = 1;
    }
}

void Entity_SetName(Entity* e, const char* name)
{
    u32 i = 0;

    memset(e->name, 0, sizeof(e->name));
    if (name) {
        while (i < OTS3_NAME_LEN - 1u && name[i] != '\0') {
            e->name[i] = name[i];
            i++;
        }
    }
}

void Entity_InitNeeds(Entity* e)
{
    u32 i;

    for (i = 0; i < NEED_COUNT; i++) {
        e->need[i] = Entity_NeedApplies(e, (u8)i) ? NEED_START_VALUE : NEED_MAX;
        e->need_rem[i] = 0;
    }
}

EntityId EntityTable_Spawn(EntityTable* t, u8 kind, u8 species, u8 household,
                           EntityId owner, const char* name, Vec2Fx pos)
{
    u32 i;

    if (kind != ENTITY_KIND_SIM && kind != ENTITY_KIND_PET) {
        return ENTITY_ID_NONE;
    }
    if ((kind == ENTITY_KIND_PET) != (species != PET_SPECIES_NONE) ||
        species >= PET_SPECIES_COUNT) {
        return ENTITY_ID_NONE;
    }
    for (i = 0; i < OTS3_MAX_ENTITIES; i++) {
        if (t->e[i].id == ENTITY_ID_NONE) {
            Entity* e = &t->e[i];
            memset(e, 0, sizeof(*e));
            e->id = ENTITY_ID_MAKE(i + 1u, t->gen[i]);
            e->owner = (kind == ENTITY_KIND_PET) ? owner : ENTITY_ID_NONE;
            e->pos = pos;
            e->kind = kind;
            e->species = species;
            e->household = household;
            e->flags = ENTITY_FLAG_ACTIVE;
            Entity_SetName(e, name);
            Entity_InitNeeds(e);
            return e->id;
        }
    }
    return ENTITY_ID_NONE;                          /* pool cheio */
}

BOOL EntityTable_Despawn(EntityTable* t, EntityId id)
{
    Entity* e = EntityTable_Get(t, id);
    u32 slot;

    if (!e) {
        return FALSE;
    }
    slot = ENTITY_ID_INDEX(id) - 1u;
    memset(e, 0, sizeof(*e));
    t->gen[slot] = (u16)(t->gen[slot] + 1u);
    if (t->gen[slot] == 0) {
        t->gen[slot] = 1;                           /* a geração 0 é reservada */
    }
    return TRUE;
}

Entity* EntityTable_Get(EntityTable* t, EntityId id)
{
    u32 idx = ENTITY_ID_INDEX(id);

    if (id == ENTITY_ID_NONE || idx == 0 || idx > OTS3_MAX_ENTITIES) {
        return NULL;
    }
    return (t->e[idx - 1u].id == id) ? &t->e[idx - 1u] : NULL;
}

const Entity* EntityTable_GetConst(const EntityTable* t, EntityId id)
{
    u32 idx = ENTITY_ID_INDEX(id);

    if (id == ENTITY_ID_NONE || idx == 0 || idx > OTS3_MAX_ENTITIES) {
        return NULL;
    }
    return (t->e[idx - 1u].id == id) ? &t->e[idx - 1u] : NULL;
}

u32 EntityTable_Count(const EntityTable* t)
{
    u32 i, n = 0;

    for (i = 0; i < OTS3_MAX_ENTITIES; i++) {
        if (t->e[i].id != ENTITY_ID_NONE) {
            n++;
        }
    }
    return n;
}

BOOL EntityTable_Validate(const EntityTable* t)
{
    u32 i, k;

    for (i = 0; i < OTS3_MAX_ENTITIES; i++) {
        const Entity* e = &t->e[i];

        if (t->gen[i] == 0) {
            return FALSE;
        }
        if (e->id == ENTITY_ID_NONE) {
            continue;
        }
        if (ENTITY_ID_INDEX(e->id) != i + 1u || ENTITY_ID_GEN(e->id) != t->gen[i]) {
            return FALSE;
        }
        if (e->kind != ENTITY_KIND_SIM && e->kind != ENTITY_KIND_PET) {
            return FALSE;
        }
        if (e->kind == ENTITY_KIND_PET) {
            if (e->species != PET_SPECIES_DOG && e->species != PET_SPECIES_CAT) {
                return FALSE;
            }
        } else if (e->species != PET_SPECIES_NONE || e->owner != ENTITY_ID_NONE) {
            return FALSE;
        }
        if (e->flags & ~ENTITY_FLAGS_MASK) {
            return FALSE;
        }
        for (k = 0; k < NEED_COUNT; k++) {
            if (e->need[k] < 0 || e->need[k] > NEED_MAX || e->need_rem[k] >= NEED_REM_DEN) {
                return FALSE;
            }
        }
        if (memchr(e->name, '\0', sizeof(e->name)) == NULL) {
            return FALSE;                           /* nome sem terminador */
        }
    }
    return TRUE;
}

void EntityRuntime_Reset(EntityRuntime* r)
{
    memset(r, 0, sizeof(*r));
    r->obj = OBJ_NONE_INDEX;
}

/* ------------------------------------------------------------ necessidades -- */

BOOL Entity_NeedApplies(const Entity* e, u8 need)
{
    return need < NEED_COUNT && DecayTable(e->kind)[need] != 0;
}

OTS3_HOT void Entity_DecayNeedsMinute(Entity* e)
{
    const u8* rate = DecayTable(e->kind);
    u32 i;

    for (i = 0; i < NEED_COUNT; i++) {
        u32 acc, dec;

        if (rate[i] == 0) {
            continue;
        }
        acc = (u32)e->need_rem[i] + rate[i];       /* taxa/h acumulada em 1/60 */
        dec = 0;
        while (acc >= NEED_REM_DEN) {              /* no máximo 4 iterações (taxa <= 255) */
            acc -= NEED_REM_DEN;
            dec++;
        }
        e->need_rem[i] = (u8)acc;
        e->need[i] = (s16)(e->need[i] > (s32)dec ? e->need[i] - (s32)dec : 0);
    }
}

void Entity_AddNeed(Entity* e, u8 need, s32 delta)
{
    s32 v;

    if (need >= NEED_COUNT) {
        return;
    }
    v = (s32)e->need[need] + delta;
    if (v < 0) {
        v = 0;
    } else if (v > NEED_MAX) {
        v = NEED_MAX;
    }
    e->need[need] = (s16)v;
}

BOOL Entity_MostUrgentNeed(const Entity* e, s16 threshold, u8* out_need)
{
    u32 i;
    BOOL found = FALSE;
    s16 best = threshold;

    for (i = 0; i < NEED_COUNT; i++) {
        if (Entity_NeedApplies(e, (u8)i) && e->need[i] < best) {
            best = e->need[i];
            *out_need = (u8)i;
            found = TRUE;
        }
    }
    return found;
}

const char* PetSpecies_Name(u8 species)
{
    switch (species) {
    case PET_SPECIES_DOG: return "dog";
    case PET_SPECIES_CAT: return "cat";
    default:              return "none";
    }
}
