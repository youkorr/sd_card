import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sd_card
from esphome.const import CONF_ID, CONF_FILES
from esphome import automation

DEPENDENCIES = ["sd_card"]
CODEOWNERS = ["@votre_nom"]

CONF_SD_CARD = "sd_card"

sd_card_ns = cg.esphome_ns.namespace('sd_card')
SDCardComponent = sd_card_ns.class_('SDCardComponent', cg.Component)
PlayMediaAction = sd_card_ns.class_('PlayMediaAction', automation.Action)

# Schéma pour les actions
STORAGE_PLAY_MEDIA_SCHEMA = cv.Schema({
    cv.Required("sd_card_id"): cv.use_id(SDCardComponent),
    cv.Required("media_file"): cv.string,
})

# Enregistrement des actions
@automation.register_action(
    "sd_card.play_media",
    PlayMediaAction,
    STORAGE_PLAY_MEDIA_SCHEMA,
)
async def sd_card_play_media_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    sd_card = await cg.get_variable(config["sd_card_id"])
    cg.add(var.set_sd_card(sd_card))
    cg.add(var.set_media_file(config["media_file"]))
    return var

# Configuration du composant SD Card
CONFIG_SCHEMA = sd_card.CONFIG_SCHEMA.extend({
    cv.Optional(CONF_FILES): cv.ensure_list({
        cv.Required("source"): cv.file_,
        cv.Required("id"): cv.string,
    }),
})

async def to_code(config):
    var = await sd_card.to_code(config)
    if CONF_FILES in config:
        for file in config[CONF_FILES]:
            cg.add(var.add_file(file["source"], file["id"]))





