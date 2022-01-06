#include<iostream>
#include<vector>
#include<array>
#include<memory>

constexpr std::size_t skill_number = 5;

struct build;
struct restraints;
struct data;

struct restraints {
    double dmg_required = 5100;
    double crit_required = 7;
    double hp_required = 0;
    double card_coins = 693;
    double guild_coins = 1320;

    double base_damage = 116.659;
    double dmg_multi = 1.5; //dagger 0.5 sword 1 axe 1.55
    double enemy_def = 58.5;
    double base_atk = 363;
    double base_crit = 1611;
};

struct data {

    data() {
        for (std::size_t i = 1; i < cost.size(); ++i) {
            total_cost.push_back(total_cost[i - 1] + cost[i]);
        }
    }

    unsigned int card_buff_coins = 0;
    unsigned int guild_buff_coins = 0;

    restraints build_restraints;

    std::array<std::size_t, skill_number> max_level = { 22,22,22,22,12 };

    std::vector<unsigned int> cost = { 0,5,6,7,8,9,11,13,15,18,21,25,29,34,40,47,55,64,75,88,104,124,150,183 };
    std::vector<unsigned int> total_cost = {0};

    std::vector<std::vector<double>> stats =
    {
        {0,2,3,4,5,6,7,8,9,11,13,15,18,21,25,29,34,40,47,55}, //crit
        {0,0.5,0.8,1.1,1.4,1.8,2.2,2.6,3.1,3.6,4.1,4.7,5.3,5.9,6.6,7.3,8,8.8,9.7,10.7,12,13.5,15,16.7}, //hp
        {0,5,6,7,8,9,11,14,18,23,29,36,44,53,63,74,86,99,113,130,150,173,200,231}, //attack
        {0,26,31,36,41,47,57,73,94,120,151,187,229,276,238,385,447,515,588,676,780,900,1035,1185}, //critdmg
        {0,7,8,9,10,11,12,14,16,18,20,22,24,26,28,30,33,36,39,42,45,49,54,60} //essence
    };
};

struct build {

    build() 
    {
    }
    build(const build&) = default;
    build& operator =(const build&) = default;

    std::array<std::size_t, skill_number> current_level_card = { 0,0,0,0,0 };

    std::array<std::size_t, skill_number> current_level_guild = { 0,0,0,0,0 };
};

struct calculator {

    data& d;
    restraints& r;

    calculator(data& d, restraints& r)
        :d(d)
        ,r(r)
    {}

    double get_dmg_only_attack(const build& b) const {
        double build_dmg = d.stats[2][b.current_level_card[2]] + d.stats[2][b.current_level_guild[2]];
        double build_crit = 0;
        double dmg = (r.base_damage * r.dmg_multi * (r.base_atk + build_dmg) / r.base_atk - r.enemy_def) * (100. + r.base_crit + build_crit) / 100.;
        return dmg;
    }

    double get_dmg(const build& b) const {
        double build_dmg = d.stats[2][b.current_level_card[2]] + d.stats[2][b.current_level_guild[2]];
        double build_crit = d.stats[3][b.current_level_card[3]] + d.stats[3][b.current_level_guild[3]];
        double dmg = (r.base_damage * r.dmg_multi * (r.base_atk + build_dmg) / r.base_atk - r.enemy_def) * (100. + r.base_crit + build_crit) / 100.;
        return dmg;
    }

    double get_essence(const build& b) const {
        return d.stats[4][b.current_level_card[4]] + d.stats[4][b.current_level_guild[4]];
    }

    double get_crit(const build& b) const {
        return d.stats[0][b.current_level_card[0]] + d.stats[0][b.current_level_guild[0]];
    }

    double get_hp(const build& b) const {
        return d.stats[1][b.current_level_card[1]] + d.stats[1][b.current_level_guild[1]];
    }

    unsigned int get_card_coin(std::size_t which, const build& b) const {
        unsigned int sum = 0;
        for (std::size_t i = 0; i < std::min(skill_number,which+1); ++i) {
            sum += d.total_cost[b.current_level_card[i]];
        }
        return sum;
    }

    unsigned int get_guild_coin(std::size_t which, const build& b) const {
        unsigned int sum = 0;
        for (std::size_t i = 0; i < std::min(skill_number, which+1); ++i) {
            sum += d.total_cost[b.current_level_guild[i]];
        }
        return sum;
    }

    unsigned int get_guild_coin(const build& b) const {
        return get_guild_coin(skill_number, b);
    }

    unsigned int get_card_coin(const build& b) const {
        return get_card_coin(skill_number, b);
    }

    unsigned int get_coin(const build& b) const {
        return get_card_coin(b) + get_guild_coin(b);
    }

    bool less(const build& b1, const build& b2) const{
        double b1_value = get_essence(b1);
        double b2_value = get_essence(b2);
        if (b1_value > b2_value) return false;
        if (b1_value < b2_value) return true;
        if (b1_value == b2_value) {
            b1_value = get_dmg(b1);
            b2_value = get_dmg(b2);
            if (b1_value > b2_value) return false;
            if (b1_value < b2_value) return true;
            if (b1_value == b2_value) {
                b1_value = get_coin(b1);
                b2_value = get_coin(b2);
                if (b1_value < b2_value) return false;
                return true;
            }
        }
        return false;
    }

    bool enough(std::size_t which, const build& b) {
        switch (which) {
        case 0:
        {
            double crit = get_crit(b);
            if (crit >= r.crit_required) return true;
            break;
        }
        case 1:
        {
            double hp = get_hp(b);
            if (hp >= r.hp_required) return true;
            break;
        }
        case 2:
        {
            double dmg = get_dmg_only_attack(b);
            if (dmg >= r.dmg_required) return true;
            break;
        }
        case 3:
        {
            double dmg = get_dmg(b);
            if (dmg >= r.dmg_required) return true;
            break;
        }
        case 4:
        {
            return false;
        }
        default:
            return false;
        }
        return false;
    }

    bool required(std::size_t which) {
        switch (which) {
            case 0u:return true;
            case 1u:return true;
            case 2u:return false;
            case 3u:return true;
            case 4u:return false;
            default:return false;
        }
    }

    bool good(const build& b) {
        if (!enough(0, b)) return false;
        if (!enough(1, b)) return false;
        if (!enough(3, b)) return false;
        return true;
    }

};

struct iterator {
    build best_build;
    build current_build;

    data& d;
    restraints& r;
    calculator& c;

    iterator(data& d, restraints& r, calculator& c)
        :d(d)
        ,r(r)
        ,c(c)
    {}
    
    void iterate(std::size_t depth) {
        if (depth >= skill_number) {
            if (c.good(current_build)) {
                if (c.less(best_build, current_build))
                    best_build = current_build;
            }
        }
        else {
            for (std::size_t i = 0; i < d.stats[depth].size(); ++i) {
                current_build.current_level_card[depth] = i;
                current_build.current_level_guild[depth] = 0;
                if (c.get_card_coin(depth, current_build) > r.card_coins) break;
                if (c.required(depth)) {
                    if (c.enough(depth, current_build)) {
                        iterate(depth + 1);
                        break;
                    }
                    else {
                        for (std::size_t j = 1; j < d.stats[depth].size(); ++j) {
                            current_build.current_level_guild[depth] = j;
                            if (c.get_guild_coin(depth, current_build) > r.guild_coins) break;
                            if (c.enough(depth, current_build)) {
                                iterate(depth + 1);
                                break;
                            }
                        }
                    }
                }
                else {
                    if (c.enough(depth, current_build)) {
                        iterate(depth + 1);
                        break;
                    }
                    for (std::size_t j = 1; j < d.stats[depth].size(); ++j) {
                        current_build.current_level_guild[depth] = j;
                        if (c.get_guild_coin(depth, current_build) > r.guild_coins) break;
                        if (c.enough(depth, current_build)) {
                            iterate(depth + 1);
                            break;
                        }
                        else
                            iterate(depth + 1);
                    }
                }
            }
        }
    }

    bool was_iterate_successful = false;

    bool iterate() {
        was_iterate_successful = false;
        iterate(0);
        return was_iterate_successful;
    }

    friend std::ostream& operator<<(std::ostream& os, iterator& it) {
        os << "card: (";
        for (std::size_t i = 0; i < skill_number; ++i) {
            os << it.best_build.current_level_card[i];
            if (i + 1 < skill_number) os << ",";
        }
        os << ") ";
        os << "guild: (";
        for (std::size_t i = 0; i < skill_number; ++i) {
            os << it.best_build.current_level_guild[i];
            if (i + 1 <= skill_number) os << ",";
        }
        os << ") damage " << it.c.get_dmg(it.best_build);
        os << " essence "<< it.c.get_essence(it.best_build);
        os << " card coin "<< it.c.get_card_coin(it.best_build);
        os << " guild coin "<< it.c.get_guild_coin(it.best_build);
        return os;
    }
};

int main()
{
    restraints r;
    data d;
    calculator c(d, r);
    build b;
    b.current_level_card = { 3,0,11,15,14 };
    b.current_level_guild = { 2,0,18,20,11 };
    build b2 = b;
    b2.current_level_card[2] = b2.current_level_guild[2] = 11;
    std::cout << c.get_dmg(b) << "\n\r";
    std::cout << c.get_coin(b) << "\n\r";
    std::cout << c.get_essence(b) << "\n\r";
    std::cout << c.good(b) << "\n\r";
    b2.current_level_guild = { 1,0,18,20,11 };
    std::cout << c.less(b,b2) <<"\n\r";
    std::cout << c.get_coin(b) << " " << c.get_coin(b2) << "\n\r";

    iterator i(d, r, c);
    i.iterate();

    std::cout << i;
}


