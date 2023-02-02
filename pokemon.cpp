#include <cstdlib>
#include <algorithm>

#include "pokemon.h"
#include "db_parse.h"

static int compare_move(const void *v1, const void *v2)
{
  return ((levelup_move *) v1)->level - ((levelup_move *) v2)->level;
}

Pokemon::Pokemon(int level) : level(level)
{
  pokemon_species_db *s;
  unsigned i, j;
  bool found;

  // Subtract 1 because array is 1-indexed
  pokemon_species_index = rand() % ((sizeof (species) /
                                     sizeof (species[0])) - 1);
  s = species + pokemon_species_index;

  ptype = rand() % 18;
  if(ptype == 0)
    type = (char*)"Normal";
  if(ptype == 1)
    type = (char*)"Fire";
  if(ptype == 2)
    type = (char*)"Water";
  if(ptype == 3)
    type = (char*)"Grass";
  if(ptype == 4)
    type = (char*)"Electric";
  if(ptype == 5)
    type = (char*)"Ice";
  if(ptype == 6)
    type = (char*)"Fighting";
  if(ptype == 7)
    type = (char*)"Poison";
  if(ptype == 8)
    type = (char*)"Ground";
  if(ptype == 9)
    type = (char*)"Flying";
  if(ptype == 10)
    type = (char*)"Psychic";
  if(ptype == 11)
    type = (char*)"Bug";
  if(ptype == 12)
    type = (char*)"Rock";
  if(ptype == 13)
    type = (char*)"Ghost";
  if(ptype == 14)
    type = (char*)"Dragon";
  if(ptype == 15)
    type = (char*)"Dark";
  if(ptype == 16)
    type = (char*)"Steel";
  if(ptype == 17)
    type = (char*)"Fairy";
  
  if (!s->levelup_moves) {
    // We have never generated a pokemon of this species before, so we
    // need to find it's level-up moveset and save it for next time.
    for (s->num_levelup_moves = 0, i = 1;
         i < (sizeof (pokemon_moves) / sizeof (pokemon_moves[0]));
         i++) {
      if (s->id == pokemon_moves[i].pokemon_id &&
          pokemon_moves[i].pokemon_move_method_id == 1) {
        for (found = false, j = 0; !found && j < s->num_levelup_moves; j++) {
          if (s->levelup_moves[j].move == pokemon_moves[i].move_id) {
            found = true;
          }
        }
        if (!found) {
          s->num_levelup_moves++;
          s->levelup_moves = ((levelup_move *)
                              realloc(s->levelup_moves,
                                      (s->num_levelup_moves *
                                       sizeof (*s->levelup_moves))));
          s->levelup_moves[s->num_levelup_moves - 1].level =
            pokemon_moves[i].level;
          s->levelup_moves[s->num_levelup_moves - 1].move =
            pokemon_moves[i].move_id;
        }
      }
    }
    // s->levelup_moves now contains all of the moves this species can learn
    // through leveling up.  Now we'll sort it by level to make that process
    // simpler.
    qsort(s->levelup_moves, s->num_levelup_moves,
          sizeof (*s->levelup_moves), compare_move);

    // Also initialize base stats while we're here
    s->base_stat[0] = pokemon_stats[pokemon_species_index * 6 - 5].base_stat;
    s->base_stat[1] = pokemon_stats[pokemon_species_index * 6 - 4].base_stat;
    s->base_stat[2] = pokemon_stats[pokemon_species_index * 6 - 3].base_stat;
    s->base_stat[3] = pokemon_stats[pokemon_species_index * 6 - 2].base_stat;
    s->base_stat[4] = pokemon_stats[pokemon_species_index * 6 - 1].base_stat;
    s->base_stat[5] = pokemon_stats[pokemon_species_index * 6 - 0].base_stat;
  }

  // Get pokemon's move(s).
  for (i = 0;
       i < s->num_levelup_moves && s->levelup_moves[i].level <= level;
       i++)
    ;

  // 0 is an invalid index, since the array is 1 indexed.
  move_index[0] = move_index[1] = move_index[2] = move_index[3] = 0;
  // I don't think 0 moves is possible, but account for it to be safe
  if (i) {
    move_index[0] = s->levelup_moves[rand() % i].move;
    if (i != 1) {
      do {
        j = rand() % i;
      } while (s->levelup_moves[j].move == move_index[0]);
      move_index[1] = s->levelup_moves[j].move;
    }
  }

  // Calculate IVs
  for (i = 0; i < 6; i++) {
    IV[i] = rand() & 0xf;
    effective_stat[i] = 5 + ((s->base_stat[i] + IV[i]) * 2 * level) / 100;
    if (i == 0) { // HP
      effective_stat[i] += 5 + level;
    }
  }
  actual_hp = effective_stat[stat_hp];
  xp = 0;
  xp_needed = experience[1].experience;

  shiny = ((rand() & 0x1fff) ? false : true);
  gender = ((rand() & 0x1fff) ? gender_female : gender_male);
}

const char *Pokemon::get_species() const
{
  return species[pokemon_species_index].identifier;
}

int Pokemon::get_lvl() const
{
  return level;
}

void Pokemon::upgrade_lvl()
{
  int i;
  double rate = (double)actual_hp / (double)effective_stat[stat_hp];
  level++;
  xp_needed = experience[level+1].experience;
  for (i = 0; i < 6; i++) {
    effective_stat[i] = 5 + ((pokemon_stats[pokemon_species_index * 6 - 5 + i].base_stat
			      + IV[i]) * 2 * level) / 100;
    if (i == 0) { // HP
      effective_stat[i] += 5 + level;
    }
  }
  actual_hp = (int)(rate*(double)effective_stat[stat_hp]);
}

int Pokemon::get_xp() const
{
  return xp;
}
int Pokemon::get_needed_xp() const
{
  return xp_needed;
}

int Pokemon::update_xp(int i)
{
  xp += i;
  if(xp >= xp_needed) {
    upgrade_lvl();
    return 1;
  }
  return 0;
}

void Pokemon::update_hp(int i)
{
  actual_hp+=i;
  if(actual_hp < 0)
    actual_hp = 0;
  if(actual_hp > effective_stat[stat_hp])
    actual_hp = effective_stat[stat_hp];
}

int Pokemon::get_hp() const
{
  return effective_stat[stat_hp];
}
int Pokemon::get_actual_hp() const
{
  return actual_hp;
}

int Pokemon::get_atk() const
{
  return effective_stat[stat_atk];
}

int Pokemon::get_def() const
{
  return effective_stat[stat_def];
}

int Pokemon::get_spatk() const
{
  return effective_stat[stat_spatk];
}

int Pokemon::get_spdef() const
{
  return effective_stat[stat_spdef];
}

int Pokemon::get_speed() const
{
  return effective_stat[stat_speed];
}

int Pokemon::get_type() const
{
  return ptype;
}

double Pokemon::get_multiplier(int p) const
{ 
  static double mp[19][19] =
    { { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,.5, 0, 1, 1,.5, 1 },//
      { 1,.5,.5, 2, 1, 2, 1, 1, 1, 1, 1, 2,.5, 1,.5, 1, 2, 1 },//
      { 1, 2,.5,.5, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1,.5, 1, 1, 1 },//
      { 1,.5, 2,.5, 1, 1, 1,.5, 2,.5, 1,.5, 2, 1,.5, 1,.5, 1 },//
      { 1, 1, 2,.5,.5, 1, 1, 1, 0, 2, 1, 1, 1, 1,.5, 1, 1, 1 },//
      { 1,.5,.5, 2, 1,.5, 1, 1, 2, 2, 1, 1, 1, 1, 2, 1,.5, 1 },//
      { 2, 1, 1, 1, 1, 2, 1,.5, 1,.5,.5,.5, 2, 0, 1, 2, 2,.5 },//
      { 1, 1, 1, 2, 1, 1, 1,.5,.5, 1, 1, 1,.5,.5, 1, 1, 0, 2 },//
      { 1, 2, 1,.5, 2, 1, 1, 2, 1, 0, 1,.5, 2, 1, 1, 1, 2, 1 },//ground
      { 1, 1, 1, 2,.5, 1, 2, 1, 1, 1, 1, 2,.5, 1, 1, 1,.5, 1 },//flying
      { 1, 1, 1, 1, 1, 1, 2, 2, 1, 1,.5, 1, 1, 1, 1, 0,.5, 1 },//psychic
      { 1,.5, 1, 2, 1, 1,.5,.5, 1,.5, 2, 1, 1,.5, 1, 2,.5,.5 },//bug
      { 1, 2, 1, 1, 1, 2,.5, 1,.5, 2, 1, 2, 1, 1, 1, 1,.5, 1 },//rock
      { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1,.5, 1, 1 },//ghost
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1,.5, 0 },//dragon
      { 1, 1, 1, 1, 1, 1,.5, 1, 1, 1, 2, 1, 1, 2, 1,.5, 1,.5 },//dark
      { 1,.5,.5, 1,.5, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,.5, 2 },
      { 1,.5, 1, 1, 1, 1, 2,.5, 1, 1, 1, 1, 1, 1, 2, 2,.5, 1 } };
  return mp[ptype][p];
}

const char *Pokemon::get_gender_string() const
{
  return gender == gender_female ? "female" : "male";
}

bool Pokemon::is_shiny() const
{
  return shiny;
}

const char *Pokemon::get_move(int i) const
{
  if (i < 4 && move_index[i]) {
    return moves[move_index[i]].identifier;
  } else {
    return "";
  }
}

int Pokemon::get_move_power(int i)
{
  if (i < 4 && move_index[i]) {
    return moves[move_index[i]].power;
  } else {
    return 1;
  }
}

int Pokemon::get_move_priority(int i)
{
  if (i < 4 && move_index[i]) {
    return moves[move_index[i]].priority;
  } else {
    return -1;
  }
}

int Pokemon::get_move_accuracy(int i)
{
  if (i < 4 && move_index[i]) {
    return moves[move_index[i]].accuracy;
  } else {
    return 1;
  }
}

std::ostream &Pokemon::print(std::ostream &o) const
{
  pokemon_species_db *s = species + pokemon_species_index;
  unsigned i;

  o << get_species() << " level:" << level << " "
    << get_gender_string() << " " << (shiny ? "shiny" : "not shiny")
    << std::endl;
  o << "         HP:" << effective_stat[stat_hp] << std::endl
    << "        ATK:" << effective_stat[stat_atk] << std::endl
    << "        DEF:" << effective_stat[stat_def] << std::endl
    << "      SPATK:" << effective_stat[stat_spatk] << std::endl
    << "      SPDEF:" << effective_stat[stat_spdef] << std::endl
    << "      SPEED:" << effective_stat[stat_speed] << std::endl;
  o << "       HPIV:" << IV[stat_hp] << std::endl
    << "      ATKIV:" << IV[stat_atk] << std::endl
    << "      DEFIV:" << IV[stat_def] << std::endl
    << "    SPATKIV:" << IV[stat_spatk] << std::endl
    << "    SPDEFIV:" << IV[stat_spdef] << std::endl
    << "    SPEEDIV:" << IV[stat_speed] << std::endl;
  o << "     HPBASE:" << s->base_stat[stat_hp] << std::endl
    << "    ATKBASE:" << s->base_stat[stat_atk] << std::endl
    << "    DEFBASE:" << s->base_stat[stat_def] << std::endl
    << "  SPATKBASE:" << s->base_stat[stat_spatk] << std::endl
    << "  SPDEFBASE:" << s->base_stat[stat_spdef] << std::endl
    << "  SPEEDBASE:" << s->base_stat[stat_speed] << std::endl;

  o << "  Levelup moves: " << std::endl; 
  for (i = 0; i < s->num_levelup_moves; i++) {
    o << "    " << moves[s->levelup_moves[i].move].identifier
      << ":" << s->levelup_moves[i].level << std::endl;
  }
  o << "  Known moves: " << std::endl;
  if (move_index[0]) {
    o << "    " << moves[move_index[0]].identifier << std::endl;
  }
  if (move_index[1]) {
    o << "    " << moves[move_index[1]].identifier << std::endl;
  }
  if (move_index[2]) {
    o << "    " << moves[move_index[2]].identifier << std::endl;
  }
  if (move_index[3]) {
    o << "    " << moves[move_index[3]].identifier << std::endl;
  }

  return o;
}

std::ostream &operator<<(std::ostream &o, const Pokemon &p)
{
  return p.print(o);
}
