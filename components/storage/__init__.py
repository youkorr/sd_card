import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PLATFORM, CONF_FILES
from esphome import automation

DEPENDENCIES = []
CODEOWNERS = ["@votre_nom"]

CONF_STORAGE = "storage"
CONF_DATA = "data"

storage_ns = cg.esphome_ns.namespace('storage')
MediaPlayerComponent = storage_ns.class_('MediaPlayerComponent', cg.Component)

# Schema for individual file
FILE_SCHEMA = cv.Schema({
    cv.Required(CONF_DATA): cv.returning_lambda,
    cv.Required(CONF_ID): cv.string,
})

# Schema for media player configuration
MEDIA_PLAYER_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(MediaPlayerComponent),
    cv.Required(CONF_PLATFORM): cv.one_of("media_player", lower=True),
    cv.Optional(CONF_FILES, default=[]): cv.ensure_list(FILE_SCHEMA),
})

# Configuration schema
CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_STORAGE): cv.ensure_list(MEDIA_PLAYER_SCHEMA)
})

def to_code(config):
    for conf in config.get(CONF_STORAGE, []):
        var = cg.new_Pvariable(conf[CONF_ID])
        yield cg.register_component(var, conf)
        
        cg.add(var.set_platform(conf[CONF_PLATFORM]))
        
        for file in conf.get(CONF_FILES, []):
            # Process lambda for data
            data_lambda = yield cg.process_lambda(
                file[CONF_DATA], 
                [],  # No arguments
                return_type=cg.std_vector(cg.uint8)
            )
            
            # Add file with processed lambda
            cg.add(var.add_file(data_lambda, file[CONF_ID]))









