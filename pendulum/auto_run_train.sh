#!/bin/bash
# Ce script est conçu pour lancer des entraînements avec différentes combinaisons de paramètres pour un grogramme (Mnist, Pendulum...).
# Il utilise des boucles pour parcourir différentes valeurs de la seed (de 0 à max_seed ), du nombre d'agents(de 2 à max_nbr_agents),
# et de la fréquence d'échange de données (de 1 à max_nbGenerationPerAggregation).

# Usage:
#   ./auto_run_train.sh max_seed_value max_nbr_agents max_nbGenerationPerAggregation
#   - max_seed_value : La valeur maximale de la seed initiale pour les entraînements.
#   - max_nbr_agents : Le nombre maximal d'agents à utiliser pour les entraînements.
#   - max_nbGenerationPerAggregation : Le nombre maximal de générations par agrégation.

# Assurez-vous d'avoir les permissions d'exécution pour ce script en utilisant la commande suivante :
#   chmod +x auto_run_train.sh

#Attetion
# Le script utilise le répertoire du script lui-même pour trouver le chemin absolu vers votre programme d'entraînement.
# Assurez-vous que le script est dans le répertoire du projet (Mnist, Pendulum...).
# Assurez-vous que l'executable soit compilé.

# Vérifier si le nombre d'arguments est correct
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 max_seed_value max_nbr_agents max_nbGenerationPerAggregation"
    exit 1
fi
# Vérifier si max_seed est un entier positif
if ! [[ "$1" =~ ^[0-9]+$ ]]; then
    echo "Erreur : max_seed doit être un entier positif."
    exit 1
fi
# Vérifier si le nombre d'agents est supérieur ou égale 2
if [ "$2" -lt 2 ]; then
    echo "nombre d'agents doit être supérieur ou égale 2"
    exit 1
fi
# Vérifier si max_nbGenerationPerAggregation est un entier positif non nul
if ! [[ "$3" =~ ^[1-9]+$ ]]; then
    echo "Erreur : max_nbGenerationPerAggregation doit être un entier positif non nul."
    exit 1
fi
max_seed="$1"  # Valeur maximale de la seed initiale
max_nbr_agents="$2" # nombre maximal d'agents
max_nbGenerationPerAggregation="$3" # nombre maximal d'agents

# Résoudre les liens symboliques et obtenir le chemin absolu du script bash
script_dir=$(dirname "$(readlink -f "$0")")
# Chemin vers votre programme
program_path="$script_dir/cmake-build-release/Release/pendulum"

#for agents in "${nb_agents[@]}"; do
# Exécuter le programme pour chaque combinaison de paramètres
for ((agent=2; agent<=max_nbr_agents; agent++)); do
  # Créer un répertoire pour stocker les fichiers de sortie
  output_dir="$script_dir/logs/FL/"$agent"_Agents"
  mkdir -p "$output_dir"

  for ((seed=0; seed<=max_seed; seed++)); do
    for ((exchange=1; exchange<=max_nbGenerationPerAggregation; exchange++)); do
        echo "Lancement du : $0 avec seed initial : $seed, nombre d'agents : $agent et freq échange de donnée : $exchange"
        "$program_path" $seed $agent $exchange
    done
  done
done

echo "Toutes les exécutions sont terminées."
