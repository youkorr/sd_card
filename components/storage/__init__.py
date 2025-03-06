import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PLATFORM
from esphome import automation

DEPENDENCIES = ['media_player']
CODEOWNERS = ["@votre_nom"]

CONF_STORAGE = "storage"
CONF_FILES = "files"
CONF_SOURCE = "source"

storage_ns = cg.esphome_ns.namespace('storage')
StorageComponent = storage_ns.class_('StorageComponent', cg.Component)

# Schema for media files
FILE_SCHEMA = cv.Schema({
    cv.Required(CONF_SOURCE): cv.string,
    cv.Required(CONF_ID): cv.string,
})

# Main storage schema
STORAGE_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(StorageComponent),
    cv.Required(CONF_PLATFORM): cv.one_of("sd_card", "flash", "inline", lower=True),
    cv.Optional(CONF_FILES, default=[]): cv.ensure_list(FILE_SCHEMA),
}).extend(cv.COMPONENT_SCHEMA)

@automation.register_action(
    "storage.play_media",
    storage_ns.class_("PlayMediaAction"),
    cv.Schema({
        cv.Required("storage_id"): cv.use_id(StorageComponent),
        cv.Required("media_file"): cv.templatable(cv.string),
        cv.Optional("announcement", default=False): cv.templatable(cv.boolean),
        cv.Optional("enqueue", default=False): cv.templatable(cv.boolean),
    })
)
def storage_play_media_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    yield cg.register_parented(var, config["storage_id"])
    
    template_ = yield cg.templatable(config["media_file"], args, str)
    cg.add(var.set_media_file(template_))
    
    template_ = yield cg.templatable(config["announcement"], args, bool)
    cg.add(var.set_announcement(template_))
    
    template_ = yield cg.templatable(config["enqueue"], args, bool)
    cg.add(var.set_enqueue(template_))
    
    yield var

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    
    cg.add(var.set_platform(config[CONF_PLATFORM]))
    
    for file in config[CONF_FILES]:
        cg.add(var.add_file(file[CONF_SOURCE], file[CONF_ID]))







