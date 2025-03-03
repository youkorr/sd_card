import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PLATFORM, CONF_FILES
from esphome import automation

DEPENDENCIES = []
CODEOWNERS = ["@votre_nom"]

CONF_STORAGE = "storage"
CONF_IMAGES = "images"
storage_ns = cg.esphome_ns.namespace('storage')
StorageComponent = storage_ns.class_('StorageComponent', cg.Component)

# Schéma pour les actions
STORAGE_PLAY_AUDIO_FILE_SCHEMA = cv.Schema({
    cv.Required("storage_id"): cv.use_id(StorageComponent),
    cv.Required("file_id"): cv.string,
})

STORAGE_LOAD_IMAGE_SCHEMA = cv.Schema({
    cv.Required("storage_id"): cv.use_id(StorageComponent),
    cv.Required("image_id"): cv.string,
})

# Enregistrement des actions
@automation.register_action(
    "storage.play_audio_file",
    storage_ns.class_("PlayAudioFileAction"),
    STORAGE_PLAY_AUDIO_FILE_SCHEMA,
)
def storage_play_audio_file_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    storage = yield cg.get_variable(config["storage_id"])
    cg.add(var.set_storage(storage))
    cg.add(var.set_file_id(config["file_id"]))
    yield var

@automation.register_action(
    "storage.load_image",
    storage_ns.class_("LoadImageAction"),
    STORAGE_LOAD_IMAGE_SCHEMA,
)
def storage_load_image_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    storage = yield cg.get_variable(config["storage_id"])
    cg.add(var.set_storage(storage))
    cg.add(var.set_image_id(config["image_id"]))
    yield var

# Schéma pour le stockage
STORAGE_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(StorageComponent),
    cv.Required(CONF_PLATFORM): cv.one_of("flash", "inline", lower=True),
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

def to_code(config):
    for conf in config:
        var = cg.new_Pvariable(conf[CONF_ID])
        yield cg.register_component(var, conf)
        
        cg.add(var.set_platform(conf[CONF_PLATFORM]))
        
        for file in conf[CONF_FILES]:
            cg.add(var.add_file(file["source"], file["id"]))
        
        if CONF_IMAGES in conf:
            for image in conf[CONF_IMAGES]:
                cg.add(var.add_image(image["file"], image["id"]))
