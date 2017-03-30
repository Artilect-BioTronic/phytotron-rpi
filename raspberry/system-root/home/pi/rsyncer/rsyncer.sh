#!/bin/bash
#########################################
# Script de sauvegarde via rsync et ssh #
#########################################

# Le fichier de configuration est fourni en argument
PROFIL="/home/pi/rsyncer/"$1".conf"

echo ""
echo "Début de la sauvegarde"
echo "######################"
echo ""
echo "Profil: $1"

EXCLUS="/home/pi/rsyncer/"$1".exclus"
source "$PROFIL"

echo "Création du répertoire de sauvegarde incrémentale:"
echo "$DESTINATION/$DATE"
mkdir --parents "$DESTINATION"/"$DATE"

# Commande de sauvegarde
rsync ${OPTIONS} --exclude-from=${EXCLUS} --backup-dir="${DESTINATION}/${DATE}" ${CIBLE} ${DESTINATION}/${DERNIER}

# Compression de l'archive désactivée pour raison de performances
#echo "Compression de l'archive..."
#tar $COMPOPTS "$DESTINATION"/"$COMPDEST" "$COMPCIBLE" && rm -rfv "$DESTINATION"/"$COMPCIBLE"

echo ""
echo "Fin de la sauvegarde"
echo "####################"

