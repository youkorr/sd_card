import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PLATFORM, CONF_FILES
from esphome import automation

DEPENDENCIES = ["sd_card"]
CODEOWNERS = ["@votre_nom"]

CONF_STORAGE = "storage"
CONF_IMAGES = "images"

storage_ns = cg.esphome_ns.namespace('storage')
StorageComponent = storage_ns.class_('StorageComponent', cg.Component)
PlayMediaAction = storage_ns.class_('PlayMediaAction', automation.Action)
LoadImageAction = storage_ns.class_('LoadImageAction', automation.Action)

# Schéma pour les actions
STORAGE_PLAY_MEDIA_SCHEMA = cv.Schema({
    cv.Required("storage_id"): cv.use_id(StorageComponent),
    cv.Required("media_file"): cv.string,
})

STORAGE_LOAD_IMAGE_SCHEMA = cv.Schema({
    cv.Required("storage_id"): cv.use_id(StorageComponent),
    cv.Required("image_id"): cv.string,
})

# Enregistrement des actions
@automation.register_action(
    "storage.play_media",
    PlayMediaAction,
    STORAGE_PLAY_MEDIA_SCHEMA,
)
async def storage_play_media_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    storage = await cg.get_variable(config["storage_id"])
    cg.add(var.set_storage(storage))
    cg.add(var.set_media_file(config["media_file"]))
    return var

@automation.register_action(
    "storage.load_image",
    LoadImageAction,
    STORAGE_LOAD_IMAGE_SCHEMA,
)
async def storage_load_image_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    storage = await cg.get_variable(config["storage_id"])
    cg.add(var.set_storage(storage))
    cg.add(var.set_image_id(config["image_id"]))
    return var

# Schéma pour le stockage
STORAGE_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(StorageComponent),
    cv.Required(CONF_PLATFORM): cv.one_of("flash", "inline", "sd_card", lower=True),
    cv.Required(CONF_FILES): cv.ensure_list({
        cv.Required("source"): cv.file_,
        cv.Required("id"): cv.string,
    }),
    cv.Optional(CONF_IMAGES): cv.ensure_list({
        cv.Required("file"): cv.file_,
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




