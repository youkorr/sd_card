import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PLATFORM
from esphome import automation

DEPENDENCIES = ['media_player']
CODEOWNERS = ["@votre_nom"]

CONF_STORAGE = "storage"
CONF_FILES = "files"
CONF_SOURCE = "source"
CONF_MEDIA_FILE = "media_file"

storage_ns = cg.esphome_ns.namespace('storage')
StorageComponent = storage_ns.class_('StorageComponent', cg.Component)

# Schema for individual file entry
FILE_SCHEMA = cv.Schema({
    cv.Required(CONF_SOURCE): cv.string,
    cv.Required(CONF_ID): cv.string,
})

# Main storage schema
CONFIG_SCHEMA = cv.Schema({
    cv.Required(CONF_PLATFORM): cv.one_of("sd_card", "flash", "inline", lower=True),
    cv.Required(CONF_ID): cv.declare_id(StorageComponent),
    cv.Required(CONF_FILES): cv.ensure_list(FILE_SCHEMA),
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    
    cg.add(var.set_platform(config[CONF_PLATFORM]))
    
    # Register each file ID globally
    for file in config[CONF_FILES]:
        cg.add(var.add_file(file[CONF_SOURCE], file[CONF_ID]))
        cg.register_variable(file[CONF_ID], var)







