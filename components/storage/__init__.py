import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PLATFORM, CONF_FILES

DEPENDENCIES = []
CODEOWNERS = ["@votre_nom"]

CONF_STORAGE = "storage"
CONF_IMAGES = "images"
storage_ns = cg.esphome_ns.namespace('storage')
StorageComponent = storage_ns.class_('StorageComponent', cg.Component)

# Ajout d'une méthode pour jouer un fichier audio
def play_audio_file(storage_id, file_id):
    return cg.statement(f"id({storage_id})->play_file({file_id});")

# Ajout d'une méthode pour accéder à une image
def load_image(storage_id, image_id):
    return cg.statement(f"id({storage_id})->load_image({image_id});")

IMAGE_SCHEMA = cv.Schema({
    cv.Required("file"): cv.string,
    cv.Required("id"): cv.string,
})

STORAGE_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(StorageComponent),
    cv.Required(CONF_PLATFORM): cv.one_of("flash", "inline", lower=True),
    cv.Required(CONF_FILES): cv.ensure_list({
        cv.Required("source"): cv.string,
        cv.Required("id"): cv.string,
    }),
    cv.Optional(CONF_IMAGES): cv.ensure_list(IMAGE_SCHEMA),
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
