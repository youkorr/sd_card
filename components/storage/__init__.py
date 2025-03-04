import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sd_card
from esphome.const import CONF_ID, CONF_PLATFORM, CONF_FILES
from esphome import automation

DEPENDENCIES = []
CODEOWNERS = ["@votre_nom"]

CONF_STORAGE = "storage"

storage_ns = cg.esphome_ns.namespace('storage')
StorageComponent = storage_ns.class_('StorageComponent', cg.Component)
PlayMediaAction = storage_ns.class_('PlayMediaAction', automation.Action)

# Schéma pour les actions
SD_CARD_PLAY_MEDIA_SCHEMA = cv.Schema({
    cv.Required("storage_id"): cv.use_id(StorageComponent),
    cv.Required("media_file"): cv.string,
})

# Enregistrement des actions
@automation.register_action(
    "sd_card.play_media",  # Nom de l'action mis à jour
    PlayMediaAction,
    SD_CARD_PLAY_MEDIA_SCHEMA,
)
async def sd_card_play_media_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    storage = await cg.get_variable(config["storage_id"])
    cg.add(var.set_storage(storage))
    cg.add(var.set_media_file(config["media_file"]))
    return var

# Schéma pour le stockage
STORAGE_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(StorageComponent),
    cv.Required(CONF_PLATFORM): cv.one_of("sd_card", lower=True),
    cv.Required(CONF_FILES): cv.ensure_list({
        cv.Required("source"): cv.file_,
        cv.Required("id"): cv.string,
    }),
}).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.All(
    cv.ensure_list(STORAGE_SCHEMA),
)

async def to_code(config):
    for conf in config:
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        
        if conf[CONF_PLATFORM] == "sd_card":
            sd_card_component = await cg.get_variable(sd_card.CONFIG_SCHEMA[CONF_ID])
            cg.add(var.set_sd_card(sd_card_component))
        
        for file in conf[CONF_FILES]:
            cg.add(var.add_file(file["source"], file["id"]))






