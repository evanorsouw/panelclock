// testcode.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "jsonparser.h"

struct Test
{
    void Run()
    {
        JsonParser parser([this](const JsonEntry& json) { return handleJson(json); });

        auto s = "{ \"liveweer\": [{\"plaats\": \"Amsterdam\", \"timestamp\": 1724006284, \"time\": \"18-08-2024 20:38:04\", \"temp\": 17.9, \"gtemp\": 16.2, \"samenv\": \"Licht bewolkt\", \"lv\": 65, \"windr\": \"NW\", \"windrgr\": 306.7, \"windms\": 3.44, \"windbft\": 3, \"windknp\": 6.7, \"windkmh\": 12.4, \"luchtd\": 1015.61, \"ldmmhg\": 762, \"dauwp\": 10.7, \"zicht\": 34600, \"gr\": 24, \"verw\": \"Droog en van het westen uit brede opklaringen. Morgen veel zon en droog\", \"sup\": \"06:28\", \"sunder\": \"21:00\", \"image\": \"lichtbewolkt\", \"alarm\": 0, \"lkop\": \"Er zijn geen waarschuwingen\", \"ltekst\": \" Er zijn momenteel geen waarschuwingen van kracht.\", \"wrschklr\": \"groen\", \"wrsch_g\": \"-\", \"wrsch_gts\": 0, \"wrsch_gc\": \"-\"}], \"wk_verw\":[{\"dag\":\"18-08-2024\",\"image\":\"halfbewolkt\",\"max_temp\":18,\"min_temp\":16,\"windbft\":2,\"windkmh\":7,\"windknp\":4,\"windms\":2,\"windrgr\":291,\"windr\":\"W\",\"neersl_perc_dag\":0,\"zond_perc_dag\":100},{\"dag\":\"19-08-2024\",\"image\":\"halfbewolkt\",\"max_temp\":25,\"min_temp\":14,\"windbft\":2,\"windkmh\":10,\"windknp\":6,\"windms\":3,\"windrgr\":212,\"windr\":\"Z\",\"neersl_perc_dag\":0,\"zond_perc_dag\":100},{\"dag\":\"20-08-2024\",\"image\":\"regen\",\"max_temp\":26,\"min_temp\":16,\"windbft\":3,\"windkmh\":14,\"windknp\":8,\"windms\":4,\"windrgr\":214,\"windr\":\"ZW\",\"neersl_perc_dag\":70,\"zond_perc_dag\":68},{\"dag\":\"21-08-2024\",\"image\":\"halfbewolkt\",\"max_temp\":18,\"min_temp\":14,\"windbft\":3,\"windkmh\":14,\"windknp\":8,\"windms\":4,\"windrgr\":226,\"windr\":\"ZW\",\"neersl_perc_dag\":20,\"zond_perc_dag\":86},{\"dag\":\"22-08-2024\",\"image\":\"halfbewolkt\",\"max_temp\":24,\"min_temp\":13,\"windbft\":3,\"windkmh\":14,\"windknp\":8,\"windms\":4,\"windrgr\":220,\"windr\":\"ZW\",\"neersl_perc_dag\":0,\"zond_perc_dag\":93}],\"uur_verw\":[{\"uur\":\"18-08-2024 20:00\",\"timestamp\":1724004000,\"image\":\"helderenacht\",\"temp\":19,\"windbft\":2,\"windkmh\":10,\"windknp\":6,\"windms\":3,\"windrgr\":318,\"windr\":\"NW\",\"neersl\":0,\"gr\":36},{\"uur\":\"18-08-2024 21:00\",\"timestamp\":1724007600,\"image\":\"helderenacht\",\"temp\":19,\"windbft\":2,\"windkmh\":10,\"windknp\":6,\"windms\":3,\"windrgr\":311,\"windr\":\"NW\",\"neersl\":0,\"gr\":0},{\"uur\":\"18-08-2024 22:00\",\"timestamp\":1724011200,\"image\":\"helderenacht\",\"temp\":19,\"windbft\":2,\"windkmh\":7,\"windknp\":4,\"windms\":2,\"windrgr\":294,\"windr\":\"W\",\"neersl\":0,\"gr\":0},{\"uur\":\"18-08-2024 23:00\",\"timestamp\":1724014800,\"image\":\"helderenacht\",\"temp\":19,\"windbft\":2,\"windkmh\":7,\"windknp\":4,\"windms\":2,\"windrgr\":276,\"windr\":\"W\",\"neersl\":0,\"gr\":0},{\"uur\":\"19-08-2024 00:00\",\"timestamp\":1724018400,\"image\":\"helderenacht\",\"temp\":19,\"windbft\":2,\"windkmh\":7,\"windknp\":4,\"windms\":2,\"windrgr\":258,\"windr\":\"W\",\"neersl\":0,\"gr\":0},{\"uur\":\"19-08-2024 01:00\",\"timestamp\":1724022000,\"image\":\"helderenacht\",\"temp\":18,\"windbft\":2,\"windkmh\":7,\"windknp\":4,\"windms\":2,\"windrgr\":235,\"windr\":\"ZW\",\"neersl\":0,\"gr\":0},{\"uur\":\"19-08-2024 02:00\",\"timestamp\":1724025600,\"image\":\"helderenacht\",\"temp\":18,\"windbft\":2,\"windkmh\":7,\"windknp\":4,\"windms\":2,\"windrgr\":221,\"windr\":\"ZW\",\"neersl\":0,\"gr\":0},{\"uur\":\"19-08-2024 03:00\",\"timestamp\":1724029200,\"image\":\"wolkennacht\",\"temp\":18,\"windbft\":2,\"windkmh\":7,\"windknp\":4,\"windms\":2,\"windrgr\":215,\"windr\":\"ZW\",\"neersl\":0,\"gr\":0},{\"uur\":\"19-08-2024 04:00\",\"timestamp\":1724032800,\"image\":\"helderenacht\",\"temp\":17,\"windbft\":2,\"windkmh\":7,\"windknp\":4,\"windms\":2,\"windrgr\":194,\"windr\":\"Z\",\"neersl\":0,\"gr\":0},{\"uur\":\"19-08-2024 05:00\",\"timestamp\":1724036400,\"image\":\"helderenacht\",\"temp\":17,\"windbft\":2,\"windkmh\":10,\"windknp\":6,\"windms\":3,\"windrgr\":190,\"windr\":\"Z\",\"neersl\":0,\"gr\":0},{\"uur\":\"19-08-2024 06:00\",\"timestamp\":1724040000,\"image\":\"helderenacht\",\"temp\":16,\"windbft\":2,\"windkmh\":10,\"windknp\":6,\"windms\":3,\"windrgr\":188,\"windr\":\"Z\",\"neersl\":0,\"gr\":6},{\"uur\":\"19-08-2024 07:00\",\"timestamp\":1724043600,\"image\":\"zonnig\",\"temp\":16,\"windbft\":2,\"windkmh\":10,\"windknp\":6,\"windms\":3,\"windrgr\":180,\"windr\":\"Z\",\"neersl\":0,\"gr\":89},{\"uur\":\"19-08-2024 08:00\",\"timestamp\":1724047200,\"image\":\"zonnig\",\"temp\":17,\"windbft\":2,\"windkmh\":10,\"windknp\":6,\"windms\":3,\"windrgr\":171,\"windr\":\"Z\",\"neersl\":0,\"gr\":233},{\"uur\":\"19-08-2024 09:00\",\"timestamp\":1724050800,\"image\":\"zonnig\",\"temp\":19,\"windbft\":2,\"windkmh\":10,\"windknp\":6,\"windms\":3,\"windrgr\":177,\"windr\":\"Z\",\"neersl\":0,\"gr\":385},{\"uur\":\"19-08-2024 10:00\",\"timestamp\":1724054400,\"image\":\"zonnig\",\"temp\":20,\"windbft\":3,\"windkmh\":14,\"windknp\":8,\"windms\":4,\"windrgr\":173,\"windr\":\"Z\",\"neersl\":0,\"gr\":526},{\"uur\":\"19-08-2024 11:00\",\"timestamp\":1724058000,\"image\":\"halfbewolkt\",\"temp\":22,\"windbft\":3,\"windkmh\":14,\"windknp\":8,\"windms\":4,\"windrgr\":181,\"windr\":\"Z\",\"neersl\":0,\"gr\":637},{\"uur\":\"19-08-2024 12:00\",\"timestamp\":1724061600,\"image\":\"zonnig\",\"temp\":23,\"windbft\":3,\"windkmh\":14,\"windknp\":8,\"windms\":4,\"windrgr\":184,\"windr\":\"Z\",\"neersl\":0,\"gr\":726},{\"uur\":\"19-08-2024 13:00\",\"timestamp\":1724065200,\"image\":\"halfbewolkt\",\"temp\":24,\"windbft\":3,\"windkmh\":14,\"windknp\":8,\"windms\":4,\"windrgr\":177,\"windr\":\"Z\",\"neersl\":0,\"gr\":759},{\"uur\":\"19-08-2024 14:00\",\"timestamp\":1724068800,\"image\":\"halfbewolkt\",\"temp\":25,\"windbft\":3,\"windkmh\":14,\"windknp\":8,\"windms\":4,\"windrgr\":167,\"windr\":\"ZO\",\"neersl\":0,\"gr\":740},{\"uur\":\"19-08-2024 15:00\",\"timestamp\":1724072400,\"image\":\"halfbewolkt\",\"temp\":26,\"windbft\":3,\"windkmh\":18,\"windknp\":10,\"windms\":5,\"windrgr\":166,\"windr\":\"ZO\",\"neersl\":0,\"gr\":679},{\"uur\":\"19-08-2024 16:00\",\"timestamp\":1724076000,\"image\":\"zonnig\",\"temp\":26,\"windbft\":3,\"windkmh\":18,\"windknp\":10,\"windms\":5,\"windrgr\":164,\"windr\":\"ZO\",\"neersl\":0,\"gr\":584},{\"uur\":\"19-08-2024 17:00\",\"timestamp\":1724079600,\"image\":\"zonnig\",\"temp\":26,\"windbft\":3,\"windkmh\":18,\"windknp\":10,\"windms\":5,\"windrgr\":158,\"windr\":\"ZO\",\"neersl\":0,\"gr\":457},{\"uur\":\"19-08-2024 18:00\",\"timestamp\":1724083200,\"image\":\"halfbewolkt\",\"temp\":25,\"windbft\":3,\"windkmh\":18,\"windknp\":10,\"windms\":5,\"windrgr\":149,\"windr\":\"ZO\",\"neersl\":0,\"gr\":302},{\"uur\":\"19-08-2024 19:00\",\"timestamp\":1724086800,\"image\":\"halfbewolkt\",\"temp\":24,\"windbft\":3,\"windkmh\":18,\"windknp\":10,\"windms\":5,\"windrgr\":145,\"windr\":\"ZO\",\"neersl\":0,\"gr\":136}],\"api\":[{\"bron\":\"Bron: Weerdata KNMI/NOAA via Weerlive.nl\",\"max_verz\":300,\"rest_verz\":0}]},true";
        parser.parse(s, strlen(s));
    }

    bool handleJson(const JsonEntry& json)
    {
        switch (json.item)
        {
        case JsonItem::Bool:
            printf("bool %s:%s\n", json.name, json.boolean ? "true" : "false");
            break;
        case JsonItem::Null:
            printf("null %s\n", json.name);
            break;
        case JsonItem::Number:
            printf("number %s:%f\n", json.name, json.number);
            break;
        case JsonItem::String:
            printf("string %s:%s\n", json.name, json.string);
            break;
        case JsonItem::Array:
            printf("%s [\n", json.name);
            break;
        case JsonItem::Object:
            printf("%s {\n", json.name);
            break;
        case JsonItem::Close:
            printf("]/}\n");
            break;
        case JsonItem::Error:
            printf("token ** ERROR **\n");
            break;
        case JsonItem::End:
            printf("stream end\n");
            break;
        }
        return true;    // continue;
    }
};

int main()
{
    Test().Run();
}
