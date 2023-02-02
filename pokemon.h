#ifndef POKEMON_H
# define POKEMON_H

# include <iostream>

enum pokemon_stat {
  stat_hp,
  stat_atk,
  stat_def,
  stat_spatk,
  stat_spdef,
  stat_speed
};

enum pokemon_gender {
  gender_female,
  gender_male
};

class Pokemon {
 private:
  int level;
  int xp_needed;
  int xp;
  int ptype;
  int pokemon_index;
  int move_index[4];
  int pokemon_species_index;
  int IV[6];
  int effective_stat[6];
  int actual_hp;
  bool shiny;
  pokemon_gender gender;
 public:
  Pokemon(int level);
  const char *get_species() const;
  char *type;
  int get_lvl() const;
  void upgrade_lvl();
  int get_xp() const;
  int get_needed_xp() const;
  int update_xp(int i);
  void update_hp(int i);
  int get_hp() const;
  int get_actual_hp() const;
  int get_atk() const;
  int get_def() const;
  int get_spatk() const;
  int get_spdef() const;
  int get_speed() const;
  int get_type() const;
  double get_multiplier(int p) const;
  const char *get_gender_string() const;
  bool is_shiny() const;
  const char *get_move(int i) const;
  int get_move_priority(int i);
  int get_move_power(int i);
  int get_move_accuracy(int i);
  std::ostream &print(std::ostream &o) const;
};

std::ostream &operator<<(std::ostream &o, const Pokemon &p);

#endif
