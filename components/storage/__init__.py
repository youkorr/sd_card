import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PLATFORM, CONF_FILES
from esphome import automation
from esphome.components import media_player

DEPENDENCIES = []
CODEOWNERS = ["@votre_nom"]

CONF_STORAGE = "storage"
CONF_IMAGES = "images"
CONF_MEDIA_PLAYER_ID = "media_player_id"

storage_ns = cg.esphome_ns.namespace('storage')
StorageComponent = storage_ns.class_('StorageComponent', cg.Component)

# Schéma pour les actions
STORAGE_PLAY_MEDIA_SCHEMA = cv.Schema({
    cv.Required(CONF_MEDIA_PLAYER_ID): cv.use_id(cv.COMPONENT_SCHEMA),
    cv.Required("storage_id"): cv.use_id(StorageComponent),
    cv.Required("media_file"): cv.string,
})

STORAGE_LOAD_IMAGE_SCHEMA = cv.Schema({
    cv.Required("storage_id"): cv.use_id(StorageComponent),
    cv.Required("image_id"): cv.string,
})

# Enregistrement des actions
@automation.register_action(
    "storage_sd_play.media",
    storage_ns.class_("PlayMediaAction"),
    STORAGE_PLAY_MEDIA_SCHEMA,
)
async def storage_play_media_to_code(config, action_id, template_arg, args):
    media_player = await cg.get_variable(config[CONF_MEDIA_PLAYER_ID])
    storage = await cg.get_variable(config["storage_id"])
    var = cg.new_Pvariable(action_id, template_arg, storage, media_player)
    cg.add(var.set_media_file(config["media_file"]))
    return var

@automation.register_action(
    "storage.load_image",
    storage_ns.class_("LoadImageAction", automation.Action),
    STORAGE_LOAD_IMAGE_SCHEMA,
)
async def storage_load_image_to_code(config, action_id, template_arg, args):
    storage = await cg.get_variable(config["storage_id"])
    var = cg.new_Pvariable(action_id, template_arg, storage)
    cg.add(var.set_image_id(config["image_id"]))
    return var

# Schéma pour le stockage
STORAGE_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(StorageComponent),
    cv.Required(CONF_PLATFORM): cv.one_of("flash", "inline", "sd_card", lower=True),
    cv.Required(CONF_FILES): cv.ensure_list({
        cv.Required("source"): cv.string,
        cv.Required("id"): cv.string,
    }),
    cv.Optional(CONF_IMAGES): cv.ensure_list({
        cv.Required("file"): cv.string,
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
        
        cg.add(var.set_platform(conf[CONF_PLATFORM]))
        
        for file in conf[CONF_FILES]:
            cg.add(var.add_file(file["source"], file["id"]))
        
        if CONF_IMAGES in conf:
            for image in conf[CONF_IMAGES]:
                cg.add(var.add_image(image["file"], image["id"]))










